OBJS += acpi/tables.o
OBJS += acpi/hook_pts.o
OBJS += acpi/patch/dsdt.o
OBJS += acpi/patch/method.o
OBJS += acpi/patch/patch_utils.o
OBJS += acpi/patch/pkg_length.o

IASL_FLAGS := -I $(ROOT_DIR)/shared/include \
	-sc -so

obj/include/asl/%.c: acpi/asl/%.asl
	@mkdir -p $(dir $@)
	$(IASL) $(IASL_FLAGS) -p $(dir $@)$* $^

obj/acpi/hook_pts.o: obj/include/asl/pts_hook.c
