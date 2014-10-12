(cl:defpackage "SB-ADB"
  (:use "CL" "SB-ALIEN")
  (:export
   ;; classes, constructors and accessors
   "ADB" "WITH-ADB"
   "DATUM" "MAKE-DATUM" "DATUM-KEY" "DATUM-DATA" "DATUM-TIMES" "DATUM-POWER"
   "RESULT" "RESULT-KEY" "RESULT-DISTANCE" "RESULT-QPOS" "RESULT-IPOS"
   "RESULTS"
   ;; functions
   "OPEN" "CLOSE" "INSERT" "RETRIEVE" "QUERY" "LISZT")
  (:shadow "OPEN" "CLOSE"))
