-- Salam link shortener - schema for MariaDB / MySQL.
--
--   mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p salam_links < schema.sql
--
-- Two tables and one deliberate duplication. links.clicks is a counter, and
-- click_log holds a row per visit, so the same number exists in two places.
-- That is on purpose: the redirect route reads only links (one indexed row,
-- one UPDATE) while the stats route aggregates the log. Every redirect pays
-- for the counter, but no redirect pays for the aggregate.
--
-- The counter is incremented with `clicks = clicks + 1`, never with a read
-- followed by a write, so simultaneous visits cannot lose a count.

CREATE TABLE IF NOT EXISTS users (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    email      VARCHAR(190) NOT NULL,
    pass_hash  VARCHAR(255) NOT NULL,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS links (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    owner_id   INT UNSIGNED NOT NULL,
    -- The short code. ASCII only and case-sensitive, so `ascii_bin`: under
    -- the database's default collation "Ab" and "aB" would collide, and a
    -- shortener that hands out both is handing out one.
    code       VARCHAR(32)  CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    target     VARCHAR(2048) NOT NULL,
    clicks     INT UNSIGNED NOT NULL DEFAULT 0,
    disabled   TINYINT(1)   NOT NULL DEFAULT 0,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_links_code (code),
    KEY idx_links_owner (owner_id, id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS click_log (
    id         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    link_id    INT UNSIGNED NOT NULL,
    clicked_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    referer    VARCHAR(255) NULL,
    PRIMARY KEY (id),
    KEY idx_clicks_link_day (link_id, clicked_at),
    CONSTRAINT fk_clicks_link FOREIGN KEY (link_id) REFERENCES links (id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
