.PHONY: srpm

.SHELLFLAGS := -xc

spec_dir := `rpm --eval "%{_specdir}"`
source_dir := `rpm --eval "%{_sourcedir}"`
srcrpm_dir := `rpm --eval "%{_srcrpmdir}"`

our_version := `git describe --tags | cut -d'-' -f1`
our_release := `git describe --tags | cut -s -d'-' -f2- | tr '-' '_'`
path_to_idl := `readlink -e .fred-idl`

project_name := fred-server
spec_file := $(project_name).spec

srpm:
	rpm -q git > /dev/null 2>&1 || dnf -y install git
	git clone https://github.com/CZ-NIC/fred-idl.git .fred-idl
	mkdir -p "$(source_dir)" && \
	mkdir -p "$(spec_dir)" && \
	git archive --prefix=$(project_name)-$(our_version)/ --format=tar.gz HEAD -o "$(source_dir)/$(project_name)-$(our_version).tar.gz"
	echo "%define our_version $(our_version)" > $(spec_dir)/$(spec_file) && \
	echo "%define our_release $(our_release)" >> $(spec_dir)/$(spec_file) && \
	cat .copr/$(spec_file) >> $(spec_dir)/$(spec_file) && \
	rpmbuild -bs $(spec_dir)/$(spec_file) --define "path_to_idl $(path_to_idl)" && \
	mv $(srcrpm_dir)/$(project_name)*.src.rpm $(outdir)
