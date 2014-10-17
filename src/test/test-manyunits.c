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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <time.h>

#include "manager.h"
#include "macro.h"
#include "util.h"

static const int NUNITS = 3000;

static const char *root_dir;

/* Cleanup the created unit files if we fail an assertion. */
#define assert_se_cleanup(expr)                                         \
        do {                                                            \
                if (_unlikely_(!(expr))) {                              \
                        cleanup_manyunits();                            \
                        log_assert_failed(#expr, __FILE__, __LINE__,    \
                                          __PRETTY_FUNCTION__);         \
                }                                                       \
        } while (false)                                                 \

static const char *many_path(int unitnum) {
        static char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s/many-%06d.service",
                 root_dir, "usr/lib/systemd/system", unitnum);
        return path;
}

static const char *link_path(int unitnum) {
        static char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s/many-%06d.service",
                 root_dir, "etc/systemd/system/some.target.wants", unitnum);
        return path;
}

static const char *another_path(void) {
        static char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s/another.service",
                 root_dir, "usr/lib/systemd/system");
        return path;
}


static void cleanup_manyunits(void) {
        int unitnum;

        fprintf(stderr, "removing %d unit files\n", NUNITS);

        for (unitnum = 0; unitnum < NUNITS; ++unitnum) {
                unlink(link_path(unitnum));
                unlink(many_path(unitnum));
        }
}

static void setup_manyunits(void) {
        int unitnum;
        const char *another;

        another = another_path();

        fprintf(stderr, "creating %d unit files\n", NUNITS);

        for (unitnum = 0; unitnum < NUNITS; ++unitnum) {
                assert_se_cleanup(link(another, many_path(unitnum)) == 0);
                assert_se_cleanup(symlink(many_path(unitnum),
                                          link_path(unitnum)) == 0);
        }
}

static void test_manyunits(void) {
        time_t t0, t1;
        int r = 0;
        int count = 0;
        Hashmap *h;
        UnitFileList *p;
        Iterator i;

        fprintf(stderr, "testing with %d unit files\n", NUNITS);

        t0 = time(NULL);
        h = hashmap_new(&string_hash_ops);
        r = unit_file_get_list(UNIT_FILE_SYSTEM, root_dir, h);
        assert_se_cleanup(r >= 0);
        HASHMAP_FOREACH(p, h, i) {
                ++count;
        }
        fprintf(stderr, "saw %d units\n", count);
        assert_se_cleanup(count == 3015);
        t1 = time(NULL);

        fprintf(stderr, "unit_file_get_list took %ld seconds\n",
                (long) (t1 - t0));

        assert_se_cleanup(t1 - t0 < 10);
}

int main(int argc, char* argv[]) {
        root_dir = strappenda(TEST_DIR, "/test-enabled-root");

        setup_manyunits();

        test_manyunits();

        cleanup_manyunits();

        return 0;
}
