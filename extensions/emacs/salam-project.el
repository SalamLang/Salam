;;; salam-project.el --- Projects, platforms and the block chain for salam-mode -*- lexical-binding: t; -*-

;; Author: The Salam Team
;; URL: https://github.com/SalamLang/Salam
;; Package-Requires: ((emacs "26.1"))
;; Keywords: languages salam tools
;; SPDX-License-Identifier: MIT

;;; Commentary:

;; Loaded by salam-mode.  It adds:
;;
;; - `salam-run' and `salam-build', which follow the project in salam.toml
;;   and the platform chosen with `salam-select-platform';
;; - `salam-new-project', which runs the `salam init' wizard in a terminal;
;; - `salam-block-chain-mode', which shows the headers around point
;;   (page > box > button) in the mode line or the header line, and
;;   `salam-goto-enclosing-block' to jump to them.

;;; Code:

(require 'subr-x)
(require 'cl-lib)
(require 'salam-mode)

(declare-function term-char-mode "term")
(declare-function make-term "term")
(defvar term-ansi-at-host)

(defcustom salam-block-chain-display 'mode-line
  "Where `salam-block-chain-mode' shows the chain of enclosing blocks.
The value is `mode-line', `header-line' or nil to show nothing."
  :type '(choice (const mode-line) (const header-line) (const :tag "Nowhere" nil))
  :group 'salam)

(defcustom salam-block-chain-separator " > "
  "String between two blocks in the chain."
  :type 'string
  :group 'salam)

(defcustom salam-block-chain-label-limit 32
  "Longest block label shown before it is cut."
  :type 'integer
  :group 'salam)

(defconst salam-platforms
  '("web" "js" "android" "ios" "linux" "macos" "windows" "all")
  "Values accepted by --platform.")

(defcustom salam-platform-file
  (expand-file-name "salam-platforms.eld" user-emacs-directory)
  "File that remembers the platform chosen for each project."
  :type 'file
  :group 'salam)

(defvar salam--platform-state 'unread
  "Alist of (PROJECT-ROOT . PLATFORM), or `unread' before the file is read.")

(defun salam--platform-state ()
  "Return the remembered platforms, reading `salam-platform-file' once."
  (when (eq salam--platform-state 'unread)
    (setq salam--platform-state
          (condition-case nil
              (with-temp-buffer
                (insert-file-contents salam-platform-file)
                (let ((data (read (current-buffer))))
                  (if (listp data) data nil)))
            (error nil))))
  salam--platform-state)

(defun salam--save-platform-state ()
  "Write the remembered platforms to `salam-platform-file'."
  (condition-case nil
      (with-temp-file salam-platform-file
        (let ((print-length nil) (print-level nil))
          (prin1 salam--platform-state (current-buffer))))
    (error nil)))

(defun salam-project-root (&optional directory)
  "Return the directory with the salam.toml above DIRECTORY, or nil."
  (let ((found (locate-dominating-file (or directory default-directory) "salam.toml")))
    (and found (file-name-as-directory (expand-file-name found)))))

(defun salam-project-platforms (&optional root)
  "Return the platforms listed under [project] in ROOT's salam.toml."
  (let ((file (expand-file-name "salam.toml" (or root (salam-project-root)))))
    (when (and root (file-readable-p file))
      (with-temp-buffer
        (insert-file-contents file)
        (goto-char (point-min))
        (when (re-search-forward "^[ \t]*\\[project\\][ \t]*$" nil t)
          (let ((limit (or (save-excursion
                             (and (re-search-forward "^[ \t]*\\[" nil t)
                                  (match-beginning 0)))
                           (point-max))))
            (when (re-search-forward "^[ \t]*platforms[ \t]*=[ \t]*\\[\\([^]]*\\)\\]" limit t)
              (let ((body (match-string 1)) (start 0) names)
                (while (string-match "\"\\([^\"]*\\)\"" body start)
                  (push (match-string 1 body) names)
                  (setq start (match-end 0)))
                (nreverse names)))))))))

(defun salam--platform-key ()
  "Return the key the platform choice is stored under."
  (or (salam-project-root) "*"))

(defun salam-platform ()
  "Return the platform chosen for the current project, or nil for none."
  (cdr (assoc (salam--platform-key) (salam--platform-state))))

(defun salam--set-platform (platform)
  "Remember PLATFORM, or forget the choice when it is nil."
  (let ((key (salam--platform-key)))
    (setq salam--platform-state
          (cl-remove key (salam--platform-state) :key #'car :test #'equal))
    (when platform
      (push (cons key platform) salam--platform-state))
    (salam--save-platform-state)))

(defun salam--platform-choices ()
  "Return the platforms to offer here."
  (let ((listed (salam-project-platforms (salam-project-root))))
    (if listed
        (append listed (and (cdr listed) '("all")))
      (copy-sequence salam-platforms))))

;;;###autoload
(defun salam-select-platform (&optional platform)
  "Choose PLATFORM for `salam-run' and `salam-build' in this project.
\"auto\" follows salam.toml again."
  (interactive)
  (let* ((choices (cons "auto" (salam--platform-choices)))
         (picked (or platform
                     (completing-read
                      (format "Salam platform (now %s): " (or (salam-platform) "auto"))
                      choices nil t nil nil (or (salam-platform) "auto")))))
    (cond ((member picked '("auto" "" nil))
           (salam--set-platform nil)
           (message "Salam platform follows salam.toml"))
          ((member picked salam-platforms)
           (salam--set-platform picked)
           (message "Salam platform: %s" picked))
          (t (user-error "Unknown platform %s" picked)))
    (force-mode-line-update t)))

(defun salam--platform-args (verb)
  "Return the --platform option for VERB, `run' or `build', or nil."
  (let ((platform (salam-platform)))
    (when (and platform
               (not (and (eq verb 'run) (member platform '("all" "js")))))
      (list (concat "--platform=" platform)))))

(defun salam--invocation (verb)
  "Return (DIRECTORY . ARGUMENTS) for VERB, `run' or `build'."
  (let ((root (salam-project-root))
        (platform (salam--platform-args verb))
        (tail (append (list "--error-style=gcc") (salam--common-args))))
    (cond
     (root
      (cons root (append (list (symbol-name verb)) platform tail)))
     ((and (eq verb 'run) (null platform))
      (cons default-directory
            (append (list salam-run-command (salam--current-file)) tail)))
     (t
      (cons default-directory
            (append (list (symbol-name verb) (salam--current-file)) platform tail))))))

(defun salam--invoke (verb)
  "Run or build through `compile' as VERB says."
  (when (buffer-modified-p)
    (when (buffer-file-name) (save-buffer)))
  (let* ((call (salam--invocation verb))
         (default-directory (car call)))
    (salam--compile (cdr call))))

;;;###autoload
(defun salam-run ()
  "Run the project, or this file.
In a project made by `salam init' this is `salam run', which serves a web
project with live reload.  The platform comes from `salam-select-platform'."
  (interactive)
  (salam--invoke 'run))

;;;###autoload
(defun salam-build ()
  "Build the project, or this file, for the chosen platform."
  (interactive)
  (salam--invoke 'build))

;;;###autoload
(defun salam-new-project (directory)
  "Run the `salam init' wizard for DIRECTORY in a terminal."
  (interactive
   (list (read-directory-name "New Salam project in: " nil nil nil)))
  (require 'term)
  (let* ((target (directory-file-name (expand-file-name directory)))
         (parent (file-name-directory target))
         (name (file-name-nondirectory target))
         (default-directory (file-name-as-directory (if (file-directory-p parent) parent default-directory)))
         (buffer (make-term "salam-init" salam-compiler nil "init" name)))
    (with-current-buffer buffer
      (term-char-mode))
    (pop-to-buffer buffer)))

(defun salam--blank-line-p ()
  "Return non-nil when the current line is empty or a comment."
  (save-excursion
    (beginning-of-line)
    (looking-at "[ \t]*\\(?:$\\|//\\|/\\*\\|\\*\\)")))

(defun salam--end-line-p ()
  "Return non-nil when the current line closes a block."
  (save-excursion
    (beginning-of-line)
    (looking-at "[ \t]*\\(?:end\\|پایان\\)[ \t]*\\(?://.*\\)?$")))

(defun salam--header-label ()
  "Return the label of the block header on this line, or nil."
  (let ((text (string-trim (buffer-substring-no-properties
                            (line-beginning-position) (line-end-position)))))
    (when (string-match "\\`\\(.*?\\)[ \t]*:[ \t]*\\(?://.*\\)?\\'" text)
      (let ((label (match-string 1 text)))
        (when (string-match "(" label)
          (setq label (string-trim-right (substring label 0 (match-beginning 0)))))
        (when (and (> (length label) salam-block-chain-label-limit))
          (setq label (concat (substring label 0 (1- salam-block-chain-label-limit)) "...")))
        (and (not (string-empty-p label)) label)))))

(defun salam-blocks (&optional position)
  "Return the blocks around POSITION as (LINE-START . LABEL), outermost first."
  (save-excursion
    (when position (goto-char position))
    (beginning-of-line)
    (let ((limit nil) (from-blank nil) chain)
      (when (salam--blank-line-p)
        (setq from-blank t)
        (while (and (not (bobp)) (salam--blank-line-p))
          (forward-line -1))
        (when (salam--blank-line-p)
          (setq limit 'none)))
      (unless (eq limit 'none)
        (cond
         ((salam--end-line-p)
          (setq limit (if from-blank
                          (current-indentation)
                        (1+ (current-indentation)))))
         (t
          (setq limit (1+ (current-indentation)))
          (let ((label (salam--header-label)))
            (when label
              (push (cons (point) label) chain)
              (setq limit (current-indentation))))))
        (while (and (> limit 0) (not (bobp)))
          (forward-line -1)
          (when (and (not (salam--blank-line-p)) (< (current-indentation) limit))
            (let ((label (salam--header-label)))
              (when label
                (push (cons (point) label) chain)
                (setq limit (current-indentation)))))))
      chain)))

(defun salam-block-chain (&optional position)
  "Return the chain of blocks around POSITION as a string, such as page > box."
  (mapconcat #'cdr (salam-blocks position) salam-block-chain-separator))

(defvar-local salam--chain-cache nil
  "Cached (TICK LINE . STRING) so redisplay does not rescan the buffer.")

(defun salam--chain-for-display ()
  "Return the chain at point, cached per line and per edit."
  (let ((tick (buffer-chars-modified-tick))
        (line (line-number-at-pos)))
    (unless (and salam--chain-cache
                 (eq (car salam--chain-cache) tick)
                 (eq (cadr salam--chain-cache) line))
      (setq salam--chain-cache
            (cons tick (cons line (condition-case nil (salam-block-chain) (error ""))))))
    (cddr salam--chain-cache)))

(defvar salam--chain-mode-line
  '(salam-block-chain-mode
    (:eval (let ((chain (salam--chain-for-display)))
             (if (string-empty-p chain) "" (concat " [" chain "]")))))
  "Mode line construct that shows the block chain.")
(put 'salam--chain-mode-line 'risky-local-variable t)

(defvar salam--chain-header-line
  '(:eval (let ((chain (salam--chain-for-display)))
            (if (string-empty-p chain) " " (concat " " chain))))
  "Header line construct that shows the block chain.")
(put 'salam--chain-header-line 'risky-local-variable t)

;;;###autoload
(define-minor-mode salam-block-chain-mode
  "Show the headers around point, such as page > box > button.
`salam-block-chain-display' says where."
  :lighter nil
  (let ((place (and salam-block-chain-mode salam-block-chain-display)))
    (setq mode-line-misc-info (remove salam--chain-mode-line mode-line-misc-info))
    (when (eq header-line-format salam--chain-header-line)
      (setq header-line-format nil))
    (cond
     ((eq place 'mode-line)
      (setq mode-line-misc-info (append mode-line-misc-info (list salam--chain-mode-line))))
     ((eq place 'header-line)
      (setq header-line-format salam--chain-header-line)))
    (if salam-block-chain-mode
        (add-hook 'which-func-functions #'salam--which-function nil t)
      (remove-hook 'which-func-functions #'salam--which-function t))
    (force-mode-line-update)))

(defun salam--which-function ()
  "Return the innermost block label for `which-function-mode'."
  (let ((blocks (salam-blocks)))
    (and blocks (cdr (car (last blocks))))))

;;;###autoload
(defun salam-goto-enclosing-block (&optional count)
  "Move to the header of the block around point.
With COUNT, go that many blocks outward."
  (interactive "p")
  (let* ((here (line-beginning-position))
         (blocks (cl-remove here (salam-blocks) :key #'car))
         (steps (max (or count 1) 1)))
    (if (null blocks)
        (user-error "Not inside a block")
      (push-mark nil t)
      (goto-char (car (nth (max (- (length blocks) steps) 0) blocks)))
      (back-to-indentation))))

;;;###autoload
(defun salam-pick-block ()
  "Choose one of the blocks around point and jump to it."
  (interactive)
  (let ((blocks (salam-blocks)))
    (unless blocks (user-error "Not inside a block"))
    (let* ((names (mapcar (lambda (block)
                            (format "%s (line %d)" (cdr block)
                                    (line-number-at-pos (car block))))
                          blocks))
           (picked (completing-read "Block: " names nil t))
           (block (nth (cl-position picked names :test #'equal) blocks)))
      (push-mark nil t)
      (goto-char (car block))
      (back-to-indentation))))

(defvar salam-mode-map)

(define-key salam-mode-map (kbd "C-c C-p") #'salam-select-platform)
(define-key salam-mode-map (kbd "C-c C-n") #'salam-new-project)
(define-key salam-mode-map (kbd "C-c C-u") #'salam-goto-enclosing-block)
(define-key salam-mode-map (kbd "C-c C-j") #'salam-pick-block)

(add-hook 'salam-mode-hook #'salam-block-chain-mode)

(provide 'salam-project)

;;; salam-project.el ends here
