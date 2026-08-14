-- Salam todo API - schema for MariaDB / MySQL.
--
--   mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p salam_todo < schema.sql
--
-- utf8mb4 throughout: a todo list that cannot hold "قهوه بخر" or an emoji is
-- not a todo list. utf8mb4_unicode_ci makes the UNIQUE on email
-- case-insensitive, which is what people expect from an email address.

CREATE TABLE IF NOT EXISTS users (
    id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
    email       VARCHAR(190) NOT NULL,
    pass_hash   VARCHAR(255) NOT NULL,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    -- 190, not 255: the index limit is 3072 bytes and utf8mb4 counts 4 per
    -- character, so 255 would not fit under older row formats.
    UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS todos (
    id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
    user_id     INT UNSIGNED NOT NULL,
    title       VARCHAR(255) NOT NULL,
    notes       TEXT         NULL,
    done        TINYINT(1)   NOT NULL DEFAULT 0,
    due_on      DATE         NULL,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    -- Every query in this API filters by user_id first, so it leads the
    -- index. `done` follows it because the list route filters on that too.
    KEY idx_todos_user_done (user_id, done),
    KEY idx_todos_user_due (user_id, due_on),
    -- ON DELETE CASCADE: closing an account takes its todos with it, in the
    -- database rather than in a delete loop the application might not finish.
    CONSTRAINT fk_todos_user FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
