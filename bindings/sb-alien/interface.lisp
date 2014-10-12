(in-package "SB-ADB")

(defclass adb ()
  ((alien :initarg alien)))
(defmethod initialize-instance :after ((o adb) &key)
  (when (and (slot-boundp o 'alien)
             (not (null-alien (slot-value o 'alien))))
    (let ((alien (slot-value o 'alien)))
      (sb-ext:finalize o (lambda () (%close alien))))))

;;; FIXME: deal with interrupt-safety / leak issues

;;; FIXME: if-does-not-exist.
(defun open (path &key (direction :input) (if-exists :error) (adb-class 'adb))
  (flet ((direction-flag (direction)
           (ecase direction
             ((:input :probe) sb-posix:o-rdonly)
             ((:output :io) sb-posix:o-rdwr))))
    (let* ((truepath (probe-file path))
           (alien (cond
                    (truepath
                     (ecase direction
                       ((:input :probe) 
                        (%%open path (direction-flag direction)))
                       ((:output :io)
                        (case if-exists
                          (:error (error "database already exists: ~S" path))
                          (:append (%open path (direction-flag direction)))
                          ;; FIXME: not the best implementation of
                          ;; :SUPERSEDE semantics ever.
                          (:supersede (delete-file path) (%create path 0 0 0))))))
                    ((eql direction :input)
                     (error "database does not exist: ~S" path))
                    (t (%create path 0 0 0)))))
      (cond
        ((null-alien alien)
         (case direction
           (:probe nil)
           (t (error "opening database failed: ~S" path))))
        (t (make-instance adb-class 'alien alien))))))
(defmethod close ((o adb))
  (when (and (slot-boundp o 'alien)
             (not (null-alien (slot-value o 'alien))))
    (%close (slot-value o 'alien))
    (sb-ext:cancel-finalization o)
    (slot-makunbound o 'alien)))
(defmacro with-adb ((adb path &rest open-args &key direction adb-class if-exists)
                    &body body)
  (declare (ignore direction adb-class if-exists))
  `(let ((,adb (open ,path ,@open-args)))
     (unwind-protect
          (locally ,@body)
       (close ,adb))))

(defgeneric l2norm (db))
(defmethod l2norm ((db adb))
  (%l2norm (slot-value db 'alien)))

(defstruct (datum
             (:constructor %make-datum)
             (:constructor 
              make-datum 
              (key %data &key times power
               &aux (data 
                     (make-array (list (length %data) (length (elt %data 0)))
                                 :element-type 'double-float 
                                 :initial-contents %data)))))
  (key (error "missing argument") :type string)
  (data (error "missing argument") :type (simple-array double-float (* *)))
  (times nil :type (or null (simple-array double-float)))
  (power nil :type (or null (simple-array double-float))))

(defgeneric insert (datum db))

(defmethod insert ((datum datum) (db adb))
  (let* ((data (datum-data datum))
         (nvectors (array-dimension data 0))
         (dim (array-dimension data 1)))
    (when (datum-times datum)
      (unless (= (array-total-size (datum-times datum)) (* 2 nvectors))
        (error "dimension mismatch for times: ~S" datum)))
    (when (datum-power datum)
      (unless (= (array-total-size (datum-power datum)) nvectors) 
        (error "dimension mismatch for power: ~S" datum)))
    (with-alien ((d adb-datum-t))
      (sb-sys:with-pinned-objects ((datum-data datum)
                                   (datum-times datum)
                                   (datum-power datum))
        (setf (slot d 'dim) dim)
        (setf (slot d 'nvectors) nvectors)
        (setf (slot d 'key) (datum-key datum))
        (setf (slot d 'data) 
              (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-data datum))))
        (if (datum-times datum)
            (setf (slot d 'times) (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-times datum))))
            (setf (slot d 'times) nil))
        (if (datum-power datum)
            (setf (slot d 'power) (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-power datum))))
            (setf (slot d 'power) nil))
        (sb-int:with-float-traps-masked (:invalid)
          (%insert-datum (slot-value db 'alien) (addr d)))))))

(defgeneric retrieve (key db))

(defmethod retrieve ((key string) (db adb))
  ;; KLUDGE: this does multiple copies of the floating point data:
  ;; once within audiodb_retrieve_datum(), and once from the alien to
  ;; the lisp arrays.  Oh well.
  (with-alien ((d adb-datum-t))
    (setf (slot d 'times) nil
          (slot d 'power) nil)
    (%retrieve-datum (slot-value db 'alien) key (addr d))
    (let* ((dim (slot d 'dim))
           (nvectors (slot d 'nvectors))
           (data (make-array (list nvectors dim) :element-type 'double-float))
           (vector (sb-ext:array-storage-vector data))
           ;; FIXME: this shares KEY
           (datum (%make-datum :key key :data data)))
      (flet ((system-area-dfloat-copy (from-sap from-offset to-sap to-offset ndfloats)
               ;; FIXME: the horror
               #+#.(cl:if (cl:= sb-vm:n-word-bits 64) '(:and) '(:or))
               (sb-kernel:system-area-ub64-copy
                from-sap from-offset to-sap to-offset ndfloats)
               #-#.(cl:if (cl:= sb-vm:n-word-bits 64) '(:and) '(:or))
               (sb-kernel:system-area-ub32-copy 
                from-sap from-offset to-sap to-offset (* 2 ndfloats))))
        (system-area-dfloat-copy (alien-sap (slot d 'data)) 0
                                 (sb-sys:vector-sap vector) 0
                                 (* dim nvectors))
        (unless (null-alien (slot d 'times))
          (let ((times (make-array (* 2 nvectors) :element-type 'double-float)))
            (system-area-dfloat-copy (alien-sap (slot d 'times)) 0
                                     (sb-sys:vector-sap times) 0
                                     (* 2 nvectors))
            (setf (datum-times datum) times)))
        (unless (null-alien (slot d 'power))
          (let ((power (make-array nvectors :element-type 'double-float)))
            (system-area-dfloat-copy (alien-sap (slot d 'power)) 0
                                     (sb-sys:vector-sap power) 0
                                     nvectors)
            (setf (datum-power datum) power)))
        (%free-datum (slot-value db 'alien) (addr d))
        datum))))

(defstruct result
  (qkey "" :type string)
  (ikey "" :type string)
  (distance 0d0 :type double-float)
  (qpos 0 :type (and unsigned-byte fixnum))
  (ipos 0 :type (and unsigned-byte fixnum)))

;;; Hrm.  To copy (from the malloc heap) or not to copy?  Copying
;;; would make things generally easier, I guess, and we have to hope
;;; that the order of magnitude is not such that the copying causes
;;; pain.
(defclass results (sequence standard-object)
  ())
(defclass copied-query-results (results)
  ((results :initarg results :accessor %copied-results)))
(defmethod print-object ((o copied-query-results) s)
  (pprint-logical-block (s nil)
    (print-unreadable-object (o s :type t)
      (format s "(~D results):~2I~@:_" (length o))
      (sequence:dosequence (r o)
        (pprint-pop)
        (format s "~A ~6,3e ~D ~D~@:_"
                (result-ikey r) (result-distance r)
                (result-qpos r) (result-ipos r))))))
      
(defmethod sequence:length ((o copied-query-results))
  (length (%copied-results o)))
(defmethod sequence:elt ((o copied-query-results) index)
  (elt (%copied-results o) index))
(defmethod (setf sequence:elt) (new-value (o copied-query-results) index)
  (setf (elt (%copied-results o) index) new-value))
(defmethod sequence:make-sequence-like 
    ((o copied-query-results) length &rest args 
     &key initial-element initial-contents)
  (declare (ignore initial-element initial-contents))
  (let ((vector (apply #'make-array length args)))
    (make-instance 'copied-query-results 'results vector)))
(defmethod sequence:adjust-sequence
    ((o copied-query-results) length &rest args
     &key initial-element initial-contents)
  (declare (ignore initial-element initial-contents))
  (let ((results (%copied-results o)))
    (apply #'sequence:adjust-sequence results length args))
  o)

(defclass proxied-query-results (results)
  ((adb :initarg adb)
   (spec :initarg spec)
   (results :initarg results)))
(defmethod initialize-instance :after ((o proxied-query-results) &key)
  (when (and (slot-boundp o 'results)
             (not (null-alien o))
             (slot-boundp o 'spec))
    (with-slots (results spec adb) o
      (flet ((results-finalizer ()
               (with-slots (alien) adb
                 (%free-query-results alien spec results))))
        (sb-ext:finalize o #'results-finalizer)))))

(defgeneric query (datum db &key))

;;; FIXME: I don't like this way of generalizing the boilerplate;
;;; isn't there a nice functional way of doing this?
(macrolet 
    ((def (name datum-class &body qdatum-forms)
       `(defmethod ,name ((datum ,datum-class) (db adb) &key 
                          (sequence-length 1) (sequence-start 0)
                          exhaustivep accumulation distance
                          ;; FIXME: dubious historical defaults
                          (npoints 10) (ntracks 10)

                          (radius nil radiusp)
                          (include-keys nil include-keys-p)
                          (exclude-keys nil exclude-keys-p)
                          (query-hop 1 query-hop-p)
                          (db-hop 1 db-hop-p))
          (unless (slot-boundp db 'alien)
            (error "database ~S is closed" db))
         (with-alien ((qid adb-query-id-t)
                      (qparams adb-query-parameters-t)
                      (qrefine adb-query-refine-t)
                      (qdatum adb-datum-t))
           ,@qdatum-forms
           (setf (slot qid 'datum) (addr qdatum))
           (setf (slot qid 'sequence-length) sequence-length)
           (setf (slot qid 'sequence-start) sequence-start)
           (setf (slot qid 'flags) (if exhaustivep 1 0))

           (setf (slot qparams 'accumulation)
                 (ecase accumulation
                   (:db 1)
                   (:per-track 2)
                   (:one-to-one 3)))
           (setf (slot qparams 'distance)
                 (ecase distance
                   (:dot-product 1)
                   (:euclidean-normed 2)
                   (:euclidean 3)))
           (setf (slot qparams 'npoints) (or npoints 0))
           (setf (slot qparams 'ntracks) (or ntracks 0))

           (let ((refine-flags 0))
             (when radiusp 
               (setf refine-flags (logior refine-flags 4))
               (setf (slot qrefine 'radius) (float radius 0d0)))
             ;; FIXME: the freeing of the KEYS slot in these
             ;; include/exclude keylists isn't interrupt-safe.
             ;;
             ;; FIXME: think quite hard about the behaviour of this
             ;; when LENGTH is 0.
             (when include-keys-p
               (setf refine-flags (logior refine-flags 1))
               (let ((length (length include-keys)))
                 (setf (slot (slot qrefine 'include) 'nkeys) length)
                 (let ((keys (make-alien c-string length)))
                   (setf (slot (slot qrefine 'include) 'keys) keys)
                   (loop for key being the elements of include-keys
                         for i upfrom 0
                         do (setf (deref keys i) key)))))
             (when exclude-keys-p
               (setf refine-flags (logior refine-flags 2))
               (let ((length (length exclude-keys)))
                 (setf (slot (slot qrefine 'exclude) 'nkeys) length)
                 (let ((keys (make-alien c-string length)))
                   (setf (slot (slot qrefine 'exclude) 'keys) keys)
                   (loop for key being the elements of exclude-keys
                         for i upfrom 0
                         do (setf (deref keys i) key)))))
             (when (or query-hop-p db-hop-p)
               (setf refine-flags (logior refine-flags 64))
               (setf (slot qrefine 'qhopsize) query-hop
                     (slot qrefine 'ihopsize) db-hop))
             (setf (slot qrefine 'flags) refine-flags))
           
           ;; FIXME: hm, this possibly suggests that there's something
           ;; a bit wrong with the C audioDB interface.  The API
           ;; currently exposed effectively requires either that all
           ;; the processing of query results happens in the same
           ;; dynamic extent as the call to audiodb_query_spec(), or
           ;; that the adb_query_spec_t object is allocated on the
           ;; heap.  We need to think harder about whether the spec
           ;; argument is really required (I think it probably isn't).
           ;;
           ;; meanwhile, here we're using it with dynamic extent anyway, so
           ;; we could put it right back on the stack.
           (let ((qspec (make-alien adb-query-spec-t)))
             (unwind-protect
                  (progn
                    (setf (slot qspec 'qid) qid)
                    (setf (slot qspec 'params) qparams)
                    (setf (slot qspec 'refine) qrefine)
                    
                    (let ((results 
                           (sb-int:with-float-traps-masked (:invalid)
                             (%query (slot-value db 'alien) qspec))))
                      (flet ((collect-copied-results ()
                               (let ((nresults (slot results 'nresults))
                                     (cresults (slot results 'results)))
                                 (coerce 
                                  (loop for i below nresults
                                        for r = (deref cresults i)
                                        collect (make-result 
                                                 :ikey (slot r 'ikey) 
                                                 :qkey (slot r 'qkey) 
                                                 :distance (slot r 'dist)
                                                 :qpos (slot r 'qpos) 
                                                 :ipos (slot r 'ipos)))
                                  'vector))))
                        (unwind-protect
                             (make-instance 'copied-query-results
                                            'results (collect-copied-results))
                          (%free-query-results (slot-value db 'alien) qspec results)))))
               (when (logbitp 0 (slot (slot qspec 'refine) 'flags))
                 (free-alien (slot (slot (slot qspec 'refine) 'include) 'keys)))
               (when (logbitp 1 (slot (slot qspec 'refine) 'flags))
                 (free-alien (slot (slot (slot qspec 'refine) 'exclude) 'keys)))
               (free-alien qspec)))))))
  (def query string (setf (slot qdatum 'key) datum
                          (slot qdatum 'data) nil))
  (def query datum 
    (setf (slot qdatum 'key) (datum-key datum))
    (setf (slot qdatum 'dim) (array-dimension (datum-data datum) 1))
    (setf (slot qdatum 'nvectors) (array-dimension (datum-data datum) 0))
    (setf (slot qdatum 'data) (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-data datum))))
    (if (datum-times datum)
        (setf (slot qdatum 'times) (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-times datum))))
        (setf (slot qdatum 'times) nil))
    (if (datum-power datum)
        (setf (slot qdatum 'power) (sb-sys:vector-sap (sb-ext:array-storage-vector (datum-times datum))))
        (setf (slot qdatum 'power) nil))))

(defgeneric liszt (adb))
(defmethod liszt ((db adb))
  (let ((results (%liszt (slot-value db 'alien))))
    (unwind-protect
         (loop for i below (slot results 'nresults)
               with entries = (slot results 'entries)
               for entry = (deref entries i)
               collect (cons (slot entry 'key) (slot entry 'nvectors)))
      (%free-liszt-results (slot-value db 'alien) results))))
      
#+test
(sb-adb:with-adb (db "/home/csr21/tmp/omras2-workshop/9.adb")
  (sb-adb:query "KSA_CHARM_337" db :exhaustivep t :sequence-length 30 
                :accumulation :per-track :distance :euclidean :npoints 1 :ntracks 20))

#+test
(sb-adb:with-adb (db "/home/csr21/tmp/omras2-workshop/9.adb")
  (sb-adb:query "KSA_CHARM_337" db :sequence-start 20 :sequence-length 20
                :accumulation :per-track :distance :euclidean-normed
                :npoints 10 :ntracks 1))

#+test
(sb-adb:with-adb (db "/home/csr21/tmp/omras2-workshop/9.adb")
  (sb-adb:query "KSA_CHARM_337" db 
                :exhaustivep t :sequence-length 30
                :accumulation :per-track :distance :euclidean-normed 
                :npoints 2 :ntracks 10))

;;; only hacks and tests below
#|
(defun foo ()
  (let ((db (%open "/home/csr21/tmp/omras2-workshop/9.adb" sb-posix:o-rdonly)))
    (unless (null-alien db)
      (unwind-protect
           (with-alien ((status adb-status-t))
           (%status db (addr status))
           (print (list (slot status 'dim) (slot status 'nfiles))))
        (%close db)))))

(defun set-up-spec (spec qid qparams qrefine)
  (declare (type (alien adb-query-parameters-t) qparams)
           (type (alien adb-query-refine-t) qrefine)
           (type (alien adb-query-id-t) qid)
           (type (alien adb-query-spec-t) spec))
  (setf (slot spec 'refine) qrefine)
  nil)

(defun bar ()
  (let ((db (%open "/home/csr21/tmp/omras2-workshop/9.adb" sb-posix:o-rdonly)))
    (unless (null-alien db)
      (unwind-protect
           (with-alien ((qid adb-query-id-t)
                        (qparams adb-query-parameters-t)
                        (qrefine adb-query-refine-t)
                        (qspec adb-query-spec-t)
                        (datum adb-datum-t))
             (setf (slot datum 'key) "KSA_CHARM_337")
             (setf (slot datum 'data) (sap-alien (sb-sys:int-sap 0) (* double)))
             
             (setf (slot qid 'datum) (addr datum))
             (setf (slot qid 'sequence-length) 30)
             (setf (slot qid 'flags) 1) ; ADB_QID_FLAG_EXHAUSTIVE
             
             (setf (slot qparams 'accumulation) 2) ; ADB_ACCUMULATION_PER_TRACK
             (setf (slot qparams 'distance) 2) ; ADB_DISTANCE_EUCLIDEAN_NORMED
             (setf (slot qparams 'npoints) 1)
             (setf (slot qparams 'ntracks) 20)
             
             (setf (slot qrefine 'flags) 0)
             (setf (slot qrefine 'hopsize) 1)
             
             (setf (slot qspec 'qid) qid)
             (setf (slot qspec 'params) qparams)
             (setf (slot qspec 'refine) qrefine)
             (let ((results (%query db (addr qspec))))
               (unless (null-alien results)
                 (unwind-protect
                      (flet ((print-result (n)
                               (let ((result (deref (slot results 'results) n)))
                                 (format t "~&~A ~F ~D ~D~%"
                                         (slot result 'key) (slot result 'dist)
                                         (slot result 'qpos) (slot result 'ipos)))))
                        (dotimes (i (slot results 'nresults))
                          (print-result i)))
                   (%free-query-results db (addr qspec) results)))))
        (%close db)))))
|#
