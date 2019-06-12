(define-module (ggg types))
(use-modules (oop goops))

(load-extension "libggg-guile.so" "scm_init_ggg")

(define-class <machine> ()
  (name #:init-keyword #:name #:accessor machine-name)
  (ethernet-address #:init-keyword #:ethernet-address #:accessor machine-ethernet-address)
  (ip-address #:init-keyword #:ip-address #:accessor machine-ip-address))

(define-class <entity> ()
  (name #:init-keyword #:name #:accessor entity-name)
  (real-name #:init-keyword #:real-name #:accessor entity-real-name)
  (home-directory #:init-keyword #:home-directory #:accessor entity-home-directory)
  (shell #:init-keyword #:shell #:accessor entity-shell)
  (id #:init-keyword #:id #:accessor entity-id))

(define-class <group> ()
  (name #:init-keyword #:name #:accessor group-name)
  (members #:init-keyword #:shell #:accessor group-members)
  (id #:init-keyword #:id #:accessor group-id #:init-value 4294967295))

(define-class <account> ()
  (name #:init-keyword #:name #:accessor account-name)
  (expiration-date #:init-keyword #:expiration-date #:accessor account-expiration-date
				   #:init-value (current-time))
  (flags #:init-keyword #:flags #:accessor account-flags #:init-value 0))

(define iso-time-format "%Y-%m-%dT%H:%M:%S%z")

(define time-point
  (lambda* (string #:optional (format iso-time-format))
		   "Convert string to UNIX timestamp"
	(car (mktime (car (strptime format string))))))

(define SUSPENDED 1)
(define PASSWORD_HAS_EXPIRED 2)

(define flags logior)

;; export all symbols
(module-map
  (lambda (sym var)
	(module-export! (current-module) (list sym)))
  (current-module))
