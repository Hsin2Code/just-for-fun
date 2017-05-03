;; P01 Find the last box of a list.

(defun my-last (lst)
  (if (null (cdr lst))
      lst
    (my-last (cdr lst))))

(defun my-last-1 (lst)
  (nthcdr (- (length lst) 1) lst))

;; P02 Find the last but one box of a list.

(defun my-but-last (lst)
  (subseq lst 0 (- (length lst) 1)))

;; P03 Find the K'th element of a list.

(defun element-at (lst n)
  (cond ((null lst) nil)
        ((= n 1) (car lst))
        (t (element-at (cdr lst) (- n 1)))))

(defun element-at-1 (lst n)
  (car (nthcdr (- n 1) lst)))

;; P04 Find the number of element of a list.

;; P05 Reverse a list.

(defun reverse-lst (lst)
  (if (null lst)
      nil
    (append (reverse-lst (cdr lst)) (list (car lst)))))

;; P06 Find out whether a list is a palindrome.

(defun palindrome (lst)
  (equal lst (reverse-lst lst)))

;; P07 Flatten a nested list structure.

(defun my-flatten (tree)
  (cond ((null tree) nil)
        ((atom tree) (list tree))
        (t (append (my-flatten (car tree))
                   (my-flatten (cdr tree))))))

;; P08 Eliminate consecutive duplicates of list elements.
(defun compress (lst)
  (if (null lst)
      lst
    (labels ((rec (lst acc prev)
                  (cond ((null lst) acc)
                        ((equal prev (car lst)) (rec (cdr lst) acc prev))
                        (t (rec (cdr lst) (cons (car lst) acc) (car lst))))))
      (reverse (rec lst nil nil)))))
(defun collect (lst)
  (if (null lst)
      nil
    (labels ((rec (lst temp acc)
                  (cond ((null lst) (cons temp acc))
                        ((equal (car lst) (car temp)) (rec (cdr lst) (cons (car lst) temp) acc))
                        (t (rec (cdr lst) (list (car lst)) (cons temp acc))))))
      (reverse (rec (cdr lst) (list (car lst)) nil)))))
my-collect

(defun general-process (combine fn lst)
  (labels ((rec (lst acc)
                (if (null lst)
                    acc
                  (rec (cdr lst) (funcall combine (funcall fn (car lst)) acc)))))
    (reverse (rec (collect lst) nil))))

(general-process #'cons #'(lambda (sublst)
                            (car sublst)) '(1 2 2 3 3 3 4 5 6 7))

;; P09 Pack consecutive duplicates of list elements into sub lists.

(general-process #'cons #'identity '(1 2 2 3 3 3 4 5 6 7))

;; P10 Run-length encoding of a list.

(defun encode (lst)
  (general-process #'cons
                   #'(lambda (sub)
                       (list (length sub) (car sub)))
                   lst))

;; P11 Modified run-length encoding.

(defun encode-modified (lst)
  (general-process #'cons
                   #'(lambda (sub)
                       (if (= 1 (length sub))
                           (car sub)
                         (list (length sub) (car sub))))
                   lst))

;; P12 Decode a run-length encoded list.

(defun decode (lst)
  (general-process #'append
                   #'(lambda (elem)
                       (let ((e (car elem)))
                         (if (atom e)
                             (list e)
                           (loop for i from 1 to (car e)
                                 collect (cadr e)))))
                   lst))

;; P13 Run-length encoding of a list (direct solution).


;; P14 Duplicate the elements of a list.

(defun general-rep (lst n)
  (mapcan #'(lambda (x)
              (loop for i from 1 to n
                    collect x))
          lst))

(defun duplicate (lst)
  (general-rep lst 2))

;; P15 Replicate the elements of a list a given number of times.

(defun repli (lst n)
  (general-rep lst n))

;; P16 Drop every N'th element from a list.


;; P17 Split a list into two parts; the length of the first part is given.
