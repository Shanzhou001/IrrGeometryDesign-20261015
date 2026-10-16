import re

with open("IrrGeometryDesign/spsshow/tspsform.ui", "r", encoding="utf-8") as f:
    content = f.read()

# Remove the trailing invalid actionQuickDemoSPS if present outside widget
content = re.sub(r'\s*<action name="actionQuickDemoSPS">.*?</action>\s*</ui>', '\n</ui>', content, flags=re.DOTALL)

# Insert actionQuickDemoSPS before </widget> (which closes TSPSForm)
if '<action name="actionQuickDemoSPS">' not in content:
    action_xml = '''  <action name="actionQuickDemoSPS">
   <property name="text">
    <string>快速载入标准示范观测系统(&amp;D)</string>
   </property>
  </action>
 </widget>'''
    content = re.sub(r'\s*</widget>\s*<layoutdefault', '\n' + action_xml + '\n <layoutdefault', content)

with open("IrrGeometryDesign/spsshow/tspsform.ui", "w", encoding="utf-8") as f:
    f.write(content)

print("Updated tspsform.ui cleanly!")
