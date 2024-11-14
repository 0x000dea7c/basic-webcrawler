(defun count-unique-disallow-lines (file-path)
  (with-temp-buffer
    (insert-file-contents file-path)
    (goto-char (point-min))
    (let ((disallow-lines (make-hash-table :test 'equal))
          (user-agent-section nil))
      (while (not (eobp))
        (let ((line (string-trim (thing-at-point 'line t))))
          (cond
           ((string-prefix-p "User-agent:" line)
            (setq user-agent-section (string= line "User-agent: *")))
           ((and user-agent-section (string-prefix-p "Disallow:" line))
            (puthash line t disallow-lines))))
        (forward-line 1))
      (hash-table-count disallow-lines))))

(let ((file-path "robots.txt"))
  (message "number of unique disallow lines for user-agent *: %d"
           (count-unique-disallow-lines file-path)))


(let ((file-path "robots.txt"))
  (format t "number of unique disallow lines for user-agent *: ~d~%"
          (count-unique-disallow-lines-from-file file-path)))
