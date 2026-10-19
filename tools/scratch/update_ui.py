with open('IrrGeometryDesign/spsshow/tspsform.ui', 'rb') as f:
    text = f.read().decode('utf-8')

# Add menu actions
m_old = """    <addaction name="actionRunSamplingBenchmark"/>
    <addaction name="actionExportAttributeReport"/>
    <addaction name="actionRunWavefieldReconstruction"/>
    <addaction name="actionRunFullExperimentSuite"/>"""

m_new = """    <addaction name="actionRunSamplingBenchmark"/>
    <addaction name="actionExportAttributeReport"/>
    <addaction name="actionRunWavefieldReconstruction"/>
    <addaction name="actionRunFullExperimentSuite"/>
    <addaction name="separator"/>
    <addaction name="actionViewGallery"/>
    <addaction name="actionOpenFiguresDir"/>"""

text = text.replace(m_old.replace('\n', '\r\n'), m_new.replace('\n', '\r\n'))
text = text.replace(m_old, m_new)

# Add action definitions
a_old = """  <action name="actionRunFullExperimentSuite">
   <property name="text">
    <string>一键运行全套科研对比试验</string>
   </property>
  </action>"""

a_new = """  <action name="actionRunFullExperimentSuite">
   <property name="text">
    <string>一键运行全套科研对比试验</string>
   </property>
  </action>
  <action name="actionViewGallery">
   <property name="text">
    <string>科研成果图件画廊与科学结论</string>
   </property>
  </action>
  <action name="actionOpenFiguresDir">
   <property name="text">
    <string>打开科研结果图件文件夹</string>
   </property>
  </action>"""

text = text.replace(a_old.replace('\n', '\r\n'), a_new.replace('\n', '\r\n'))
text = text.replace(a_old, a_new)

lines = text.splitlines()
normalized = '\r\n'.join(lines) + '\r\n'
with open('IrrGeometryDesign/spsshow/tspsform.ui', 'wb') as f:
    f.write(normalized.encode('utf-8'))
print('tspsform.ui updated successfully')
