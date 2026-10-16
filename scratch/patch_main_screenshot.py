with open("IrrGeometryDesign/main.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Add includes
includes_str = '''#include "tspsform.h"
#include "geometryexperimentengine.h"
#include "spsshow/tspsdlg.h"
#include "spsshow/tcellgridform.h"
#include "spssampledlg.h"
#include "tchangeshowareadlg.h"
#include "trealovtdlg.h"'''

text = text.replace('#include "tspsform.h"\n#include "geometryexperimentengine.h"', includes_str)

old_screenshot_pat = '''	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot") == 0)
		{
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("current_gui_window.png");
			return 0;
		}
	}'''

new_screenshot_pat = '''	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--screenshot") == 0)
		{
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("current_gui_window.png");
			return 0;
		}
		if (strcmp(argv[i], "--screenshot-all") == 0)
		{
			QCoreApplication::processEvents();
			QPixmap pix = QPixmap::grabWidget(&w);
			pix.save("audit_main_window.png");

			// 1. TSPSDlg
			TSPSDlg dlg1;
			dlg1.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg1).save("audit_dlg_tspsdlg.png");
			dlg1.close();

			// 2. TCellGridForm
			TCellGridForm dlg2;
			dlg2.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg2).save("audit_dlg_tcellgridform.png");
			dlg2.close();

			// 3. SPSSampleDlg
			SPSSampleDlg dlg3;
			dlg3.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg3).save("audit_dlg_spssampledlg.png");
			dlg3.close();

			// 4. TChangeShowareadlg
			TChangeShowareadlg dlg4;
			dlg4.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg4).save("audit_dlg_tchangeshowareadlg.png");
			dlg4.close();

			// 5. TRealOVTdlg
			TRealOVTdlg dlg5;
			dlg5.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg5).save("audit_dlg_trealovtdlg.png");
			dlg5.close();

			// 6. ResearchFiguresDialog
			ResearchFiguresDialog dlg6;
			dlg6.show();
			QCoreApplication::processEvents();
			QPixmap::grabWidget(&dlg6).save("audit_dlg_researchfigures.png");
			dlg6.close();

			printf("Captured all audit screenshots successfully!\\n");
			return 0;
		}
	}'''

text = text.replace(old_screenshot_pat, new_screenshot_pat)

with open("IrrGeometryDesign/main.cpp", "wb") as f:
    f.write(b"\xef\xbb\xbf" + text.encode("utf-8"))

print("Updated main.cpp with --screenshot-all!")
