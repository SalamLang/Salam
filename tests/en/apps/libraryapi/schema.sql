-- Salam library API - schema for MariaDB / MySQL.
--
--   mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p salam_library < schema.sql
--
-- The interesting column is books.copies_available. It is derived data - you
-- could compute it as copies_total minus the open loans - and it is stored
-- anyway, because it is what makes "borrow" a single conditional UPDATE that
-- two simultaneous requests cannot both win. See _store.salam's Borrow.
--
-- The CHECK constraint is the backstop: if any code path ever gets the
-- arithmetic wrong, the database refuses the write rather than quietly
-- lending out a book that is not there.

CREATE TABLE IF NOT EXISTS users (
    id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
    email       VARCHAR(190) NOT NULL,
    pass_hash   VARCHAR(255) NOT NULL,
    role        ENUM('member','librarian') NOT NULL DEFAULT 'member',
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS books (
    id               INT UNSIGNED NOT NULL AUTO_INCREMENT,
    isbn             VARCHAR(20)  NOT NULL,
    title            VARCHAR(255) NOT NULL,
    author           VARCHAR(190) NOT NULL,
    published_year   SMALLINT UNSIGNED NULL,
    copies_total     SMALLINT UNSIGNED NOT NULL DEFAULT 1,
    copies_available SMALLINT UNSIGNED NOT NULL DEFAULT 1,
    created_at       DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_books_isbn (isbn),
    KEY idx_books_author (author),
    KEY idx_books_available (copies_available),
    CONSTRAINT ck_books_copies CHECK (copies_available <= copies_total)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS loans (
    id          INT UNSIGNED NOT NULL AUTO_INCREMENT,
    book_id     INT UNSIGNED NOT NULL,
    user_id     INT UNSIGNED NOT NULL,
    borrowed_at DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    due_at      DATE         NOT NULL,
    returned_at DATETIME     NULL,
    PRIMARY KEY (id),
    -- The index that answers "what does this member have out" and "is this
    -- one overdue", which are the only two questions asked of this table.
    KEY idx_loans_user_open (user_id, returned_at),
    KEY idx_loans_due (returned_at, due_at),
    KEY idx_loans_book (book_id, returned_at),
    CONSTRAINT fk_loans_book FOREIGN KEY (book_id) REFERENCES books (id) ON DELETE RESTRICT,
    CONSTRAINT fk_loans_user FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- A few books to borrow. INSERT IGNORE so the file can be re-run.
INSERT IGNORE INTO books (isbn, title, author, published_year, copies_total, copies_available) VALUES
    ('9780262033848', 'Introduction to Algorithms', 'Cormen, Leiserson, Rivest, Stein', 2009, 3, 3),
    ('9780131103627', 'The C Programming Language',  'Kernighan and Ritchie',           1988, 2, 2),
    ('9780201616224', 'The Pragmatic Programmer',    'Hunt and Thomas',                 1999, 1, 1),
    ('9789644480102', 'بوف کور',                      'صادق هدایت',                       1936, 1, 1);
