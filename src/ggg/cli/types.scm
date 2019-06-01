(use-modules (oop goops))

(define-class <machine> ()
  (name #:init-keyword #:name #:accessor machine-name)
  (ethernet-address #:init-keyword #:ethernet-address #:accessor machine-ethernet-address)
  (ip-address #:init-keyword #:ip-address #:accessor machine-ip-address))

(define-class <entity> ()
  (name #:init-keyword #:name #:accessor entity-name)
  (real-name #:init-keyword #:real-name #:accessor entity-real-name)
  (home-directory #:init-keyword #:home-directory #:accessor entity-home-directory
				  #:init-value "")
  (shell #:init-keyword #:shell #:accessor entity-shell #:init-value "/bin/sh")
  (id #:init-keyword #:id #:accessor entity-id #:init-value 4294967295))

(define-class <group> ()
  (name #:init-keyword #:name #:accessor group-name)
  (members #:init-keyword #:shell #:accessor group-members)
  (id #:init-keyword #:id #:accessor group-id #:init-value 4294967295))
