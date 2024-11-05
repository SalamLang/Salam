.PHONY: check
check: checkinstall
	pre-commit run --all-files

.PHONY: checkinstall
checkinstall:
	pre-commit install

.PHONY: checkupdate
checkupdate: checkinstall
	pre-commit autoupdate

.PHONY: install
install:
	pip install -r requirements.txt
