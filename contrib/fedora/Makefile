.PHONY: srpm

.SHELLFLAGS := -xc

spec_dir := $(shell rpm --eval "%{_specdir}")
source_dir := $(shell rpm --eval "%{_sourcedir}")
srcrpm_dir := $(shell rpm --eval "%{_srcrpmdir}")

our_version := `git describe --tags | cut -d'-' -f1`
our_release := `git describe --tags | cut -s -d'-' -f2- | tr '-' '_'`

idl_branch := master
clone_limit := 32
project_name := fred-server
spec_file := $(project_name).spec

srpm:
	rpm -q git > /dev/null 2>&1 || dnf -y install git
	git clone --depth $(clone_limit) --no-single-branch -b $(idl_branch) https://github.com/CZ-NIC/fred-idl.git .fred-idl
	cd .fred-idl && git archive --prefix=idl-$(idl_branch)/ --format=tar.gz HEAD -o "$(source_dir)/idl-$(idl_branch).tar.gz"
	mkdir -p "$(source_dir)" && \
	mkdir -p "$(spec_dir)" && \
	git archive --prefix=$(project_name)-$(our_version)/ --format=tar.gz HEAD -o "$(source_dir)/$(project_name)-$(our_version).tar.gz" && \
	echo "%define project_name $(project_name)" > $(spec_dir)/$(spec_file) && \
	echo "%define our_version $(our_version)" >> $(spec_dir)/$(spec_file) && \
	if [[ -n "$(our_release)" ]]; then echo "%define our_release $(our_release)" >> $(spec_dir)/$(spec_file); fi && \
	echo "%define idl_branch $(idl_branch)" >> $(spec_dir)/$(spec_file) && \
	cat .copr/$(spec_file) >> $(spec_dir)/$(spec_file) && \
	rpmbuild -bs $(spec_dir)/$(spec_file) && \
	mv $(srcrpm_dir)/$(project_name)*.src.rpm $(outdir)
