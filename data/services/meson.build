xxd = find_program('xxd.sh')

service_xml = custom_target('service_xml',
  input: 'com.github.fragoi.Autobright.xml',
  output: 'com_github_fragoi_Autobright_xml.h',
  command: [xxd, '@CURRENT_SOURCE_DIR@', '@PLAINNAME@'],
  capture: true)
