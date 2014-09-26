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

static void test_enabled(int argc, char* argv[]) {

    const char *subdir = "/test-enabled-root";
    char root_dir[UNIT_NAME_MAX + 2 + 1] = TEST_DIR;

    strncat(root_dir, subdir, strlen(subdir));

    assert_se(unit_file_get_state(UNIT_FILE_SYSTEM, root_dir, "another.service") == UNIT_FILE_ENABLED);

}

int main(int argc, char* argv[]) {
        test_enabled(argc, argv);
        return 0;
}
