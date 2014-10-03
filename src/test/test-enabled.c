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

/*
.
├── etc
│   └── systemd
│       └── system
│           ├── masked.service -> /dev/null
│           ├── maskedstatic.service -> /dev/null
│           ├── some.target
│           └── some.target.wants
│               ├── another.service -> ../../../../usr/lib/systemd/system/another.service
│               ├── aliased.service -> ../../../../usr/lib/systemd/system/another.service
│               ├── different.service -> ../../../../usr/lib/systemd/system/unique.service
│               ├── masked.service -> ../../../../usr/lib/systemd/system/masked.service
│               ├── also_masked.service -> ../../../../usr/lib/systemd/system/masked.service
│               ├── templating@one.service -> ../../../../usr/lib/systemd/system/templating@.service
│               ├── templating@two.service -> ../../../../usr/lib/systemd/system/templating@two.service
│               ├── templating@three.service -> ../../../../usr/lib/systemd/system/templating@.service
│               └── templating@four.service -> ../../../../usr/lib/systemd/system/templating@four.service
├── run
│   └── systemd
│       └── system
│           ├── maskedruntime.service -> /dev/null
│           ├── maskedruntimestatic.service -> /dev/null
│           ├── other.target
│           └── other.target.wants
│               └── runtime.service -> ../../../../usr/lib/systemd/system/runtime.service
└── usr
    └── lib
        └── systemd
            └── system
                ├── invalid.service
                ├── disabled.service
                ├── another.service
                ├── runtime.service
                ├── masked.service
                ├── maskedruntime.service
                ├── static.service
                ├── maskedstatic.service
                ├── maskedruntimestatic.service
                ├── templating@.service
                ├── templating@two.service
                ├── templating@three.service
                └── unique.service
*/


char root_dir[UNIT_NAME_MAX + 2 + 1] = TEST_DIR;

#define confirm_unit_state(unit, expected)                              \
        assert_se(unit_file_get_state(UNIT_FILE_SYSTEM, root_dir, unit, ec) == expected)

static void test_enabled(int argc, char* argv[], EnabledContext *ec) {
        Hashmap *h;
        UnitFileList *p;
        Iterator i;
        int r;

        /* Explicitly check each of the units. */
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

        /* Reconcile unit_file_get_list with the return for each unit. */
        h = hashmap_new(string_hash_func, string_compare_func);
        r = unit_file_get_list(UNIT_FILE_SYSTEM, root_dir, h, ec);
        assert_se(r == 0);
        HASHMAP_FOREACH(p, h, i) {
                UnitFileState s;

                s = unit_file_get_state(UNIT_FILE_SYSTEM, root_dir,
                                        basename(p->path), ec);

                /* unit_file_get_list and unit_file_get_state are
                 * a little different in some cases.  Handle these
                 * cases here ...
                 */
                switch ((int)s) {
                case UNIT_FILE_ENABLED_RUNTIME:
                        assert_se(p->state == UNIT_FILE_ENABLED);
                        break;
                case -EBADMSG:
                        assert_se(p->state == UNIT_FILE_INVALID);
                        break;
                default:
                        assert_se(p->state == s);
                        break;
                }

                fprintf(stderr, "%s (%s)\n",
                        p->path,
                        unit_file_state_to_string(p->state));
        }
        unit_file_list_free(h);
}

const char *subdir = "/test-enabled-root";

int main(int argc, char* argv[]) {
        _cleanup_enabled_context_ EnabledContext *ec = NULL;

        strncat(root_dir, subdir, strlen(subdir));

        /* built-in EnabledContext */
        // test_enabled(argc, argv, NULL);

        /* explicit EnabledContext */
        ec = enabled_context_new();
        assert(ec);
        test_enabled(argc, argv, ec);

        return 0;
}
