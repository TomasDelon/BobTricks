.PHONY: help build-native run run-native install-hooks forge-prepare forge-check forge-publish forge-autopublish

help:
	@echo "Available targets:"
	@echo "  make help"
	@echo "  make build-native"
	@echo "  make run"
	@echo "  make run-native"
	@echo "  make install-hooks"
	@echo "  make forge-prepare COMMITS=\"<sha1> <sha2> ...\""
	@echo "  make forge-check"
	@echo "  make forge-publish"
	@echo "  make forge-autopublish"

build-native:
	@./scripts/build/build_native.sh

run: run-native

run-native:
	@./scripts/dev/run_native.sh

install-hooks:
	@./scripts/dev/install_git_hooks.sh

forge-prepare:
	@if [ -z "$(COMMITS)" ]; then echo "Set COMMITS=\"<sha1> <sha2> ...\""; exit 1; fi
	@./scripts/dev/forge_prepare_batch.sh $(COMMITS)

forge-check:
	@./scripts/dev/forge_check_batch.sh

forge-publish:
	@./scripts/dev/forge_publish_batch.sh

forge-autopublish:
	@./scripts/dev/forge_autopublish.sh
