doxygen_cmd := doxygen
doxygen_cfg := cocada.doxy
head_license_cmd := headlicense.sh
fmt_cmd := astyle
fmt_options := --style=kr --indent=tab --recursive -n


# documentation
.PHONY: doc clean-doc
doc:
	$(doxygen_cmd) $(doxygen_cfg)


clean_doc:
	$(rmdir) $(doc_dir)	

# code formatting 

fmt:
	$(head_license_cmd) $(filter-out $(thrdpty_paths), $(src_paths)) $(filter-out $(thrdpty_paths), $(head_paths)) $(test_src_paths) $(test_head_paths)
	$(fmt_cmd) $(fmt_options) $(src_dir)/*.$(src_ext),*.$(head_ext) $(test_dir)/*.$(src_ext),*.$(head_ext)
	