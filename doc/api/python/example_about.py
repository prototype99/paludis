#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

"""A simple example showing how to use Paludis version constants"""

import paludis

print(f"Built using Paludis "
      f"{paludis.VERSION_MAJOR}."
      f"{paludis.VERSION_MINOR}."
      f"{paludis.VERSION_MICRO}{paludis.VERSION_SUFFIX}"
      )

if paludis.GIT_HEAD:
    print(f"git-{paludis.GIT_HEAD}")
