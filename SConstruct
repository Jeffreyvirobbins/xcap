#
# Top-level SConstruct file.
#

env = Environment()

# You need to change this path according to where your boost installed
env.Append (CPPFLAGS='-std=c++11 -DBOOST_LOG_DYN_LINK')
# //env.Append (CPPPATH=['/usr/local', '/home/vagrant/diameter/lib/include'])
env.Append (CPPPATH=['/home/vagrant/diameter/lib/include'])
env.Append (CXXFLAGS=['-Wno-deprecated', '-Wall'])
# env.Append (LIBSPATH=['/home/vagrant/Downloads/boost_1_69_0/stage/lib','/usr/lib64/mysql'])
# new directory is located at /usr/local/lib','/usr/lib64/mysql'])
# env.Append (LIBSPATH=['/usr/lib64/mysql'])
env.Append (LIBS=['mysqlclient', 'pthread', 'boost_regex', 'boost_log', 'boost_log_setup', 'boost_filesystem', 'boost_system', 'boost_thread', 'boost_chrono', 'boost_container'])
#env.Append (LIBS=['pthread', 'boost_regex', 'boost_log', 'boost_log_setup', 'boost_filesystem', 'boost_system', 'boost_thread', 'boost_chrono', 'boost_container'])
Export ('env')

#library = env.SConscript ('lib/src/SConscript', variant_dir='lib/object', duplicated=0)
# env.Install ('lib/lib', library)


http3 = env.SConscript ('http/src/http3/SConscript', variant_dir='http/object/http3', duplicated=0)
env.Install ('http/bin', http3)


# Install to the system by root user
# env.Install ('/usr/local/lib', diameter)
# env.Install ('/usr/local/bin', server)
# env.Alias ('install', '/usr/local/bin')
