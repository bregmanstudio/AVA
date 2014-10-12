(in-package "SB-ADB")

(defmacro define-int-checking-function (name arglist llname)
  `(defun ,name ,arglist
     (let ((result (,llname ,@arglist)))
       (unless (eql 0 result)
         (error "~@<~S on ~{~S ~}failed.~@:>" ',name (list ,@arglist))))))

(defmacro define-pointer-checking-function (name arglist llname)
  `(defun ,name ,arglist
     (let ((result (,llname ,@arglist)))
       (when (null-alien result)
         (error "~@<~S on ~{~S ~}failed.~@:>" ',name (list ,@arglist)))
       result)))

(define-alien-type adb-t
  (struct adb))

(define-alien-routine ("audiodb_open" %%open) (* adb-t)
  (path c-string)
  (flags int))
(define-pointer-checking-function %open (path flags) %%open)

(define-alien-routine ("audiodb_create" %%create) (* adb-t)
  (path c-string)
  (datasize (unsigned 32))
  (ntracks (unsigned 32))
  (datadim (unsigned 32)))
(define-pointer-checking-function %create (path datasize ntracks datadim) 
  %%create)

(define-alien-routine ("audiodb_l2norm" %%l2norm) int
  (adb (* adb-t)))
(define-int-checking-function %l2norm (adb) %%l2norm)

(define-alien-routine ("audiodb_power" %%power) int
  (adb (* adb-t)))
(define-int-checking-function %power (adb) %%power)

(define-alien-type adb-datum-t
  (struct adb-datum
    (nvectors (unsigned 32))
    (dim (unsigned 32))
    (key c-string)
    (data (* double))
    (power (* double))
    (times (* double))))

(define-alien-routine ("audiodb_insert_datum" %%insert-datum) int
  (adb (* adb-t))
  (datum (* adb-datum-t)))
(define-int-checking-function %insert-datum (adb datum) %%insert-datum)

(define-alien-routine ("audiodb_retrieve_datum" %%retrieve-datum) int
  (adb (* adb-t))
  (key c-string)
  (datum (* adb-datum-t)))
(define-int-checking-function %retrieve-datum (adb key datum) %%retrieve-datum)

(define-alien-routine ("audiodb_free_datum" %%free-datum) int
  (adb (* adb-t))
  (datum (* adb-datum-t)))
(define-int-checking-function %free-datum (adb datum) %%free-datum)

(define-alien-type adb-status-t
  (struct adb-status
    (nfiles (unsigned 32))
    (dim (unsigned 32))
    (ignore1 (unsigned 32))
    (ignore2 (unsigned 32))
    (flags (unsigned 32))
    (length (unsigned 64))
    (data-region-size (unsigned 64))))

(define-alien-routine ("audiodb_status" %%status) int
  (adb (* adb-t))
  (status (* adb-status-t)))
(define-int-checking-function %status (adb datum) %%status)

(define-alien-type adb-query-id-t
  (struct adb-query-id
    (datum (* adb-datum-t))
    (sequence-length (unsigned 32))
    (flags (unsigned 32))
    (sequence-start (unsigned 32))))

(define-alien-type adb-query-parameters-t
  (struct adb-query-parameters
    (accumulation (unsigned 32))
    (distance (unsigned 32))
    (npoints (unsigned 32))
    (ntracks (unsigned 32))))

(define-alien-type adb-keylist-t
  (struct adb-key-list
    (nkeys (unsigned 32))
    (keys (* c-string))))

(define-alien-type adb-query-refine-t
  (struct adb-query-refine
    (flags (unsigned 32))
    (include adb-keylist-t)
    (exclude adb-keylist-t)
    (radius double)
    (absolute-threshold double)
    (relative-threshold double)
    (duration-ratio double)
    (qhopsize (unsigned 32))
    (ihopsize (unsigned 32))))

(define-alien-type adb-query-spec-t
  (struct adb-query-spec
    (qid adb-query-id-t)
    (params adb-query-parameters-t)
    (refine adb-query-refine-t)))

(define-alien-type adb-result-t
  (struct adb-result
    (qkey c-string)
    (ikey c-string)
    (qpos (unsigned 32))
    (ipos (unsigned 32))
    (dist double)))

(define-alien-type adb-query-results-t
  (struct adb-query-results
    (nresults (unsigned 32))
    (results (* adb-result-t))))

(define-alien-routine ("audiodb_query_spec" %%query) (* adb-query-results-t)
  (adb (* adb-t))
  (spec (* adb-query-spec-t)))
(define-pointer-checking-function %query (adb spec) %%query)

(define-alien-routine ("audiodb_query_free_results" %%free-query-results) int
  (adb (* adb-t))
  (spec (* adb-query-spec-t))
  (results (* adb-query-results-t)))
(define-int-checking-function %free-query-results (adb spec results)
  %%free-query-results)

(define-alien-type adb-track-entry-t
  (struct adb-track-entry
    (nvectors (unsigned 32))
    (key c-string)))

(define-alien-type adb-liszt-results-t
  (struct adb-liszt-results
    (nresults (unsigned 32))
    (entries (* adb-track-entry-t))))

(define-alien-routine ("audiodb_liszt" %%liszt) (* adb-liszt-results-t)
  (adb (* adb-t)))
(define-pointer-checking-function %liszt (adb) %%liszt)
(define-alien-routine ("audiodb_liszt_free_results" %%free-liszt-results) int
  (adb (* adb-t))
  (results (* adb-liszt-results-t)))
(define-int-checking-function %free-liszt-results (adb results)
  %%free-liszt-results)

(define-alien-routine ("audiodb_close" %close) void
  (adb (* adb-t)))
