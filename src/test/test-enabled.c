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

static void confirm_state(const char *unit, UnitFileState expected_state) {
        UnitFileState actual_state;

        actual_state = unit_file_get_state(UNIT_FILE_SYSTEM, root_dir, unit);

        assert_se(actual_state == expected_state);
}

static void confirm_error(const char *unit, int expected_errno) {
        int actual_return;

        actual_return = unit_file_get_state(UNIT_FILE_SYSTEM, root_dir, unit);

        assert_se(actual_return == -expected_errno);
}

static void test_enabled(int argc, char* argv[]) {

        strncat(root_dir, subdir, strlen(subdir));

        confirm_error("disabled.service", ENOENT);
        confirm_state("another.service", UNIT_FILE_ENABLED);
        confirm_state("aliased.service", UNIT_FILE_ENABLED);
        confirm_state("different.service", UNIT_FILE_ENABLED);
        confirm_state("unique.service", UNIT_FILE_ENABLED);
        confirm_state("masked.service", UNIT_FILE_DISABLED);
        confirm_state("also_masked.service", UNIT_FILE_DISABLED);
        confirm_state("masked.service", UNIT_FILE_DISABLED);

        confirm_state("templatating@.service", UNIT_FILE_ENABLED);
        confirm_state("templatating@one.service", UNIT_FILE_ENABLED);
        confirm_state("templatating@two.service", UNIT_FILE_ENABLED);
        confirm_state("templatating@three.service", UNIT_FILE_ENABLED);
        confirm_state("templatating@four.service", UNIT_FILE_DISABLED);
}

int main(int argc, char* argv[]) {
        test_enabled(argc, argv);
        return 0;
}
