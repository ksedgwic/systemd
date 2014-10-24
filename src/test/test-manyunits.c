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

#include "manager.h"
#include "macro.h"
#include "util.h"

#define NUNITS 3000

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

static char *many_path(int unitnum) {
        int r;
        char *path;
        r = asprintf(&path, "%s/%s/many-%06d.service", root_dir, "usr/lib/systemd/system", unitnum);
        assert_se(r >= 0);
        return path;
}

static char *link_path(int unitnum) {
        int r;
        char *path;
        r = asprintf(&path, "%s/%s/many-%06d.service", root_dir, "etc/systemd/system/some.target.wants", unitnum);
        assert_se(r >= 0);
        return path;
}

static char *another_path(void) {
        int r;
        char *path;
        r = asprintf(&path, "%s/%s/another.service", root_dir, "usr/lib/systemd/system");
        assert_se(r >= 0);
        return path;
}


static void cleanup_manyunits(void) {
        int unitnum;

        fprintf(stderr, "removing %d unit files\n", NUNITS);

        for (unitnum = 0; unitnum < NUNITS; ++unitnum) {
                _cleanup_free_ char *lpath = NULL;
                _cleanup_free_ char *mpath = NULL;
                lpath = link_path(unitnum);
                mpath = many_path(unitnum);
                unlink(lpath);
                unlink(mpath);
        }
}

static void setup_manyunits(void) {
        int unitnum;
        _cleanup_free_ char *apath = NULL;

        apath = another_path();

        fprintf(stderr, "creating %d unit files\n", NUNITS);

        for (unitnum = 0; unitnum < NUNITS; ++unitnum) {
                _cleanup_free_ char *lpath = NULL;
                _cleanup_free_ char *mpath = NULL;
                lpath = link_path(unitnum);
                mpath = many_path(unitnum);
                assert_se_cleanup(link(apath, mpath) == 0);
                assert_se_cleanup(symlink(mpath, lpath) == 0);
        }
}

static void test_manyunits(EnabledContext *ec) {
        usec_t t0, t1;
        int r = 0;
        int count = 0;
        Hashmap *h;
        UnitFileList *p;
        Iterator i;

        log_info("testing with %d unit files\n", NUNITS);

        t0 = now(CLOCK_MONOTONIC);
        h = hashmap_new(&string_hash_ops);
        assert_se_cleanup(h);
        r = unit_file_get_list(UNIT_FILE_SYSTEM, root_dir, h, ec);
        assert_se_cleanup(r >= 0);
        HASHMAP_FOREACH(p, h, i)
                ++count;
        log_info("saw %d units\n", count);
        assert_se_cleanup(count == 3015);
        t1 = now(CLOCK_MONOTONIC);

        log_info("unit_file_get_list took %f seconds\n", (t1 - t0) / 1e6);

        /* dangerous should these tests run on some overworked build
         * system host.
         *
         * assert_se_cleanup(t1 - t0 < (10 * 1e6));
         */
}

int main(int argc, char* argv[]) {
        _cleanup_enabled_context_ EnabledContext *ec = NULL;

        root_dir = strappenda(TEST_DIR, "/test-enabled-root");

        setup_manyunits();

        ec = enabled_context_new();
        assert(ec);

        test_manyunits(ec);

        cleanup_manyunits();

        return 0;
}
