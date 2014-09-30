/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

/***
  This file is part of systemd.

  Copyright 2014 Pantheon, Inc.

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "util.h"
#include "path-util.h"
#include "unit-name.h"
#include "install.h"

const char *subdir = "/test-enabled-root";
char root_dir[UNIT_NAME_MAX + 2 + 1] = TEST_DIR;

#define confirm_unit_state(unit, expected)                              \
        assert_se(unit_file_get_state(UNIT_FILE_SYSTEM, root_dir, unit) == expected)

static void test_enabled(int argc, char* argv[]) {

        strncat(root_dir, subdir, strlen(subdir));

        confirm_unit_state("nonexistent.service",	-ENOENT);
        confirm_unit_state("invalid.service", 		-EBADMSG);
        confirm_unit_state("disabled.service", 		UNIT_FILE_DISABLED);
        confirm_unit_state("another.service", 		UNIT_FILE_ENABLED);
        confirm_unit_state("runtime.service", 		UNIT_FILE_ENABLED_RUNTIME);
        confirm_unit_state("masked.service", 		UNIT_FILE_MASKED);
        confirm_unit_state("maskedruntime.service",	UNIT_FILE_MASKED_RUNTIME);
        confirm_unit_state("static.service", 		UNIT_FILE_STATIC);
        confirm_unit_state("maskedstatic.service",	UNIT_FILE_MASKED);
        confirm_unit_state("maskedruntimestatic.service", UNIT_FILE_MASKED_RUNTIME);
        confirm_unit_state("templating@.service",	UNIT_FILE_ENABLED);
        confirm_unit_state("templating@two.service",	UNIT_FILE_ENABLED);
        confirm_unit_state("templating@three.service",	UNIT_FILE_ENABLED);
        confirm_unit_state("unique.service", 		UNIT_FILE_ENABLED);
}

int main(int argc, char* argv[]) {
        test_enabled(argc, argv);
        return 0;
}
