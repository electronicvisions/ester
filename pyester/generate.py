#!/usr/bin/env python
import sys,os
import logging

logging.basicConfig(level=logging.INFO)

from pywrap.wrapper import Wrapper
from pywrap import namespaces

wrap = Wrapper()
module_name = wrap.module_name()
mb = wrap.mb

c = mb.class_("Server")
c.include()

c = mb.class_("PyEster")
c.include()
createFactory = c.mem_funs('create')
c.add_fake_constructors( createFactory )

# Exclude boost::serialization and boost::archive
ns_boost = mb.global_ns.namespace('boost')
ns_boost.namespace('serialization').exclude()
ns_boost.namespace('archive').exclude()

# expose only public interfaces
namespaces.exclude_by_access_type(mb, ['variables', 'calldefs', 'classes'], 'private')
namespaces.exclude_by_access_type(mb, ['variables', 'calldefs', 'classes'], 'protected')

wrap.set_number_of_files(0)
wrap.finish()
