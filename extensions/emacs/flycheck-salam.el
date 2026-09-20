;;; flycheck-salam.el --- Flycheck support for Salam -*- lexical-binding: t; -*-

;; Author: The Salam Team
;; URL: https://github.com/SalamLang/Salam
;; Package-Requires: ((emacs "26.1") (flycheck "32") (salam-mode "0.4.0"))
;; Keywords: languages salam tools
;; SPDX-License-Identifier: MIT

;;; Commentary:

;; Add (require 'flycheck-salam) after flycheck and salam-mode.

;;; Code:

(require 'flycheck)
(require 'salam-mode)

(defun flycheck-salam--null-device ()
  "Return the platform's bit bucket."
  (if (memq system-type '(windows-nt ms-dos)) "NUL" "/dev/null"))

(flycheck-define-checker salam
  "A Salam syntax and type checker.

Runs `salam inspect', which lexes, parses and analyzes the file without
running it or writing any artifact."
  :command ((eval salam-compiler)
            "inspect"
            source
            "--emit-symbol-xml"
            (eval (concat "--xml-out=" (flycheck-salam--null-device)))
            "--error-style=gcc"
            "--log-level=error"
            (eval (concat "--lang=" (salam--detect-lang))))
  :error-patterns
  ((error line-start (file-name) ":" line ":" column ": error: " (message) line-end)
   (warning line-start (file-name) ":" line ":" column ": warning: " (message) line-end)
   (info line-start (file-name) ":" line ":" column ": note: " (message) line-end))
  :modes salam-mode)

(add-to-list 'flycheck-checkers 'salam)

(provide 'flycheck-salam)

;;; flycheck-salam.el ends here
