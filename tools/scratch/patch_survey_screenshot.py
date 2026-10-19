with open("IrrGeometryDesign/spsshow/tspsform.h", "r", encoding="utf-8") as f:
    text_h = f.read()

text_h = text_h.replace("void quickLoadDemoSPS();", "void quickLoadDemoSPS(bool showMsgBox = true);")

with open("IrrGeometryDesign/spsshow/tspsform.h", "wb") as f:
    f.write(b"\xef\xbb\xbf" + text_h.encode("utf-8"))

with open("IrrGeometryDesign/spsshow/tspsform.cpp", "r", encoding="utf-8") as f:
    text_cpp = f.read()

text_cpp = text_cpp.replace("void TSPSForm::quickLoadDemoSPS()\n{", "void TSPSForm::quickLoadDemoSPS(bool showMsgBox)\n{")
text_cpp = text_cpp.replace(
    'QMessageBox::information(this,\n        QString::fromLocal8Bit("示范工区加载成功"),',
    'if (showMsgBox) {\n    QMessageBox::information(this,\n        QString::fromLocal8Bit("示范工区加载成功"),'
)
# close if statement
text_cpp = text_cpp.replace(
    '科研成果图件！"));\n}',
    '科研成果图件！"));\n    }\n}'
)

with open("IrrGeometryDesign/spsshow/tspsform.cpp", "wb") as f:
    f.write(b"\xef\xbb\xbf" + text_cpp.encode("utf-8"))

# In main.cpp, add --screenshot-survey
with open("IrrGeometryDesign/main.cpp", "r", encoding="utf-8") as f:
    text_main = f.read()

text_main = text_main.replace(
    'return a.exec();',
    '''for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot-survey") == 0)
		{
			w.quickLoadDemoSPS(false);
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("audit_populated_survey.png");
			return 0;
		}
	}

	return a.exec();'''
)

with open("IrrGeometryDesign/main.cpp", "wb") as f:
    f.write(b"\xef\xbb\xbf" + text_main.encode("utf-8"))

print("Patched quickLoadDemoSPS and main.cpp successfully!")
