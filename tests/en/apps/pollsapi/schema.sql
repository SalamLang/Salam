-- Salam polls API - schema for MariaDB / MySQL.
--
--   mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p salam_polls < schema.sql
--
-- The line that matters is UNIQUE KEY uq_votes_one_per_user (poll_id,
-- user_id). "One vote each" is not a rule the application can enforce on its
-- own: checking for an existing vote and then inserting one leaves a window
-- where two requests both see nothing and both insert. The index closes it -
-- the second INSERT fails, and the API turns that failure into a 409.
--
-- Vote counts are NOT stored. They are a GROUP BY over this table, which
-- cannot disagree with the votes the way a cached counter can.

CREATE TABLE IF NOT EXISTS users (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    email      VARCHAR(190) NOT NULL,
    pass_hash  VARCHAR(255) NOT NULL,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS polls (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    owner_id   INT UNSIGNED NOT NULL,
    question   VARCHAR(255) NOT NULL,
    closes_at  DATETIME     NULL,
    closed     TINYINT(1)   NOT NULL DEFAULT 0,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    KEY idx_polls_owner (owner_id, id),
    CONSTRAINT fk_polls_owner FOREIGN KEY (owner_id) REFERENCES users (id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS options (
    id       INT UNSIGNED NOT NULL AUTO_INCREMENT,
    poll_id  INT UNSIGNED NOT NULL,
    label    VARCHAR(190) NOT NULL,
    position TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (id),
    KEY idx_options_poll (poll_id, position),
    CONSTRAINT fk_options_poll FOREIGN KEY (poll_id) REFERENCES polls (id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS votes (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    poll_id    INT UNSIGNED NOT NULL,
    option_id  INT UNSIGNED NOT NULL,
    user_id    INT UNSIGNED NOT NULL,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    -- The whole ballot box, in one constraint.
    UNIQUE KEY uq_votes_one_per_user (poll_id, user_id),
    KEY idx_votes_tally (poll_id, option_id),
    CONSTRAINT fk_votes_poll FOREIGN KEY (poll_id) REFERENCES polls (id) ON DELETE CASCADE,
    CONSTRAINT fk_votes_option FOREIGN KEY (option_id) REFERENCES options (id) ON DELETE CASCADE,
    CONSTRAINT fk_votes_user FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
