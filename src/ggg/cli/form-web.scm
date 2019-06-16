(use-modules (web server)
			 (web request)
			 (web response)
			 (web uri)
			 (sxml simple)
			 (rnrs bytevectors)
			 (srfi srfi-1)
			 (ice-9 format)
			 (oop goops)
			 (ggg types)
			 ((ggg forms) #:prefix ggg/))

(setlocale LC_ALL "")
(define re-name (make-regexp "^[a-zа-я '\\-]+$" regexp/icase))
(define re-username (make-regexp "^[a-z]([a-z0-9_\\-]|\\.){1,16}[a-z0-9]$" regexp/icase))
(define %entropy 30.0)
(define %shell "/bin/bash")
(define %home-prefix "/home")

(define (templatize title body)
  `(html (@ (lang "ru"))
		 (head
		   (meta (@ (charset "utf-8")))
		   (meta (@ (name "viewport")
					(content "width=device-width, initial-scale=1, shrink-to-fit=no")))
		   (title ,title)
		   (link (@ (rel "stylesheet")
					(href "https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css")
					(integrity "sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm")
					(crossorigin "anonymous"))))
		 (body ,@body)))

(define*
  (respond #:optional body #:key
		   (status 200)
		   (title "Hello hello!")
		   (doctype "<!DOCTYPE html>\n")
		   (content-type-params '((charset . "utf-8")))
		   (content-type 'text/html)
		   (extra-headers '())
		   (sxml (and body (templatize title body))))
  (values (build-response
			#:code status
			#:headers `((content-type
						  . (,content-type ,@content-type-params))
						,@extra-headers))
		  (lambda (port)
			(if sxml
				(begin
				  (if doctype (display doctype port))
				  (sxml->xml sxml port))))))

(define (form-to-list data)
  (map (lambda (pair) (map uri-decode (string-split pair #\=)))
	   (string-split data #\&)))

(define* (form-field name list #:optional (defaultValue ""))
  (let ((value (assoc name list)))
	(if value (car (cdr value)) defaultValue)))

(define (input-classes value valid)
  (let ((classes (list "form-control")))
	(if value
		(if valid
			(append classes (list "is-valid"))
			(append classes (list "is-invalid")))
		classes)))

(define (validate-name value data) (regexp-exec re-name value))
(define (validate-username value data) (ggg/validate-username value re-username))
(define (validate-password value data) (ggg/validate-password value %entropy))
(define (validate-repeated-password value data)
  (string=? (form-field "password" data "") value))

(define-class <field> ()
  (name #:init-keyword #:name #:accessor field-name)
  (label #:init-keyword #:label #:accessor field-label)
  (value #:init-keyword #:value #:accessor field-value #:init-value "")
  (validate #:init-keyword #:validate #:accessor field-validate)
  (valid #:init-keyword #:valid #:accessor field-valid #:init-value #f)
  (type #:init-keyword #:type #:accessor field-type #:init-value "text")
  (error #:init-keyword #:error #:accessor field-error #:init-value ""))

(define-method (input-value (field <field>))
  (let ((id (field-name field))
		(value (field-value field)))
	`((div (@ (class "form-group"))
		   (label (@ (for ,id)) ,(field-label field))
		   (input (@ (type ,(field-type field))
					 (class ,(string-join (input-classes value (field-valid field)) " "))
					 (id ,id)
					 (name ,id)
					 (value ,(if value value ""))))
		   (div (@ (class "invalid-feedback")) ,(field-error field))))))


(define %fields
  (list
	(make <field> #:name "firstName" #:label "Имя" #:validate validate-name)
	(make <field> #:name "lastName" #:label "Фамилия" #:validate validate-name)
	(make <field> #:name "userName" #:label "Имя пользователя" #:validate validate-username)
	(make <field>
		  #:name "password"
		  #:label "Пароль"
		  #:validate validate-password
		  #:type "password")
	(make <field>
		  #:name "passwordRepeat"
		  #:label "Пароль (повторно)" 
		  #:validate validate-repeated-password
		  #:type "password")))

(define (copy-and-validate data)
  (if (null? data)
	  (list)
	  (map (lambda (field)
			 (let* ((validate (field-validate field))
					(name (field-name field))
					(value (form-field name data #f))
					(new-field (shallow-clone field)))
			   (begin
				 (set! (field-valid new-field) (validate value data))
				 (set! (field-value new-field) value)
				 new-field)))
		   %fields)))

(define (register title fields)
  (define (find-field name)
	(find (lambda (field) (string=? (field-name field) name)) fields))
  (define (find-value name) (field-value (find-field name)))
  (let ((first-name (find-value "firstName"))
		(last-name (find-value "lastName"))
		(username (find-value "userName"))
		(password (find-value "password"))
		(date (localtime (current-time))))
	(begin
	  ;; calculate expiration date
	  (set-tm:sec date 0)
	  (set-tm:min date 0)
	  (set-tm:hour date 0)
	  (set-tm:mday date 1)
	  (if (< (tm:mon date) 6)
		  (set-tm:mon date 6)
		  (begin
			(set-tm:mon date 1)
			(set-tm:year date (+ (tm:year date) 1))))
	  (catch 'ggg-error
			 (lambda ()
			   (ggg-entity-insert
				 (make <entity>
					   #:name username
					   #:real-name (string-join (list last-name first-name) " ")
					   #:home-directory (string-join (list %home-prefix username) "/")
					   #:shell %shell))
			   (ggg-account-insert
				 (make <account>
					   #:name username
					   #:password (password-hash password)
					   #:expiration-date (car (mktime date))))
			   (success-page title))
			 (lambda (key . parameters)
			   (set! (field-error (find-field "userName"))
				 (format #f "~a\n" (last parameters)))
			   (form-page title fields))))))

(define (form-page title fields)
  (respond
	`((div (@ (class "container"))
		(h1 ,title)
		(form (@ (action "") (method "POST")
				 (enctype "application/x-www-form-urlencoded"))
			  ,@(map input-value fields)
			  (button (@ (type "submit") (class "btn btn-primary")) "Зарегистрироваться!"))))
	#:title title))

(define (success-page title)
  (respond
	`((div (@ (class "container"))
		(div (@ (class "text-center mt-2"))
			 (h1 "Регистрация прошла успешно!")
			 (img (@ (class "rounded mt")
					 (src "https://source.unsplash.com/640x480/?animal"))))))
	#:title title))

(define (debug-page request body)
  (define data (if body (form-to-list (utf8->string body)) (list)))
  (define title "Регистрация")
  (define fields (copy-and-validate data))
  (if (eq? (request-method request) 'POST)
	  (if (map field-valid fields)
		  (register title fields)
		  (form-page title fields))
	  (form-page title fields)))

(run-server debug-page 'http `(#:port 8080))
