-- Salam blog API - schema for MariaDB / MySQL.
--
--   mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p salam_blog < schema.sql
--
-- Three tables, two relationships, and the interesting part is what the
-- database refuses rather than what it stores: a slug is unique, a comment
-- cannot outlive its post, and an author cannot be deleted out from under
-- their work.

CREATE TABLE IF NOT EXISTS users (
    id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
    email       VARCHAR(190) NOT NULL,
    pass_hash   VARCHAR(255) NOT NULL,
    -- 'author' or 'admin'. An ENUM rather than a string because the set is
    -- closed and the database should say so; adding a role later is a
    -- migration, which is the correct amount of friction for a permission.
    role        ENUM('author','admin') NOT NULL DEFAULT 'author',
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS posts (
    id           INT UNSIGNED NOT NULL AUTO_INCREMENT,
    author_id    INT UNSIGNED NOT NULL,
    slug         VARCHAR(190) NOT NULL,
    title        VARCHAR(255) NOT NULL,
    body         MEDIUMTEXT   NOT NULL,
    published    TINYINT(1)   NOT NULL DEFAULT 0,
    published_at DATETIME     NULL,
    created_at   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    -- The public reads posts by slug, so it is unique and indexed. Two posts
    -- called "Hello world" cannot both own /posts/hello-world, and the
    -- database is the only place that can enforce that without a race.
    UNIQUE KEY uq_posts_slug (slug),
    KEY idx_posts_public (published, published_at),
    KEY idx_posts_author (author_id, created_at),
    -- RESTRICT, not CASCADE: deleting a user must not silently delete their
    -- posts. Reassign or unpublish them first, deliberately.
    CONSTRAINT fk_posts_author FOREIGN KEY (author_id) REFERENCES users (id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS comments (
    id         INT UNSIGNED NOT NULL AUTO_INCREMENT,
    post_id    INT UNSIGNED NOT NULL,
    author_id  INT UNSIGNED NOT NULL,
    body       TEXT         NOT NULL,
    created_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    KEY idx_comments_post (post_id, id),
    -- CASCADE here: a comment on a deleted post is not data, it is a leak.
    CONSTRAINT fk_comments_post FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE,
    CONSTRAINT fk_comments_author FOREIGN KEY (author_id) REFERENCES users (id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
