with open('IrrGeometryDesign/geometryexperimentengine.cpp', 'r', encoding='utf-8-sig') as f:
    text = f.read()

text = text.replace('#include "scientificplotter.h"\n', '')
text = '#include "scientificplotter.h"\n' + text

with open('IrrGeometryDesign/geometryexperimentengine.cpp', 'w', encoding='utf-8-sig') as f:
    f.write(text)

print("Moved scientificplotter.h to line 1.")
