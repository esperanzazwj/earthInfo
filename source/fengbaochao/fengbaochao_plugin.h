/**
 * @file ship_plugin.h
 * 
 * @breif Sample plugin to show the basic plugin structure.
 */
#ifndef _FBC_PLUGIN_H
#define _FBC_PLUGIN_H

#include <string>
using namespace std;

#include "plugin_base.h"
#include "namespace.h"
#include "request_dispatcher.h"
#include "Objects/manualObject.h"
#include "fengbaochao.h"

// forward decl.
class FengBaoChao;

using namespace VirtualGlobeScene;

BEGIN_PLUGIN_NAMESPACE

class ManualNode : public ManualObject
{
public:
	virtual void draw(){
		if(NULL != fbc){
			fbc->draw();
		}
	}

	FengBaoChao *fbc;
};

class FengBaoChaoPlugin : public PluginBase
{
    Q_OBJECT;
    Q_PLUGIN_METADATA(IID "com.oceanInfo.FengBaoChaoPlugin" FILE "fengbaochao.json");
    Q_INTERFACES(PluginInterface);

public:
    FengBaoChaoPlugin();
    virtual ~FengBaoChaoPlugin();
    virtual void Load(const QDir& plugins_dir, RenderView* render_view=0);
    virtual void Unload();
	virtual void update();
    virtual std::string OnJsRequest(const std::string& message);
	void switchTaifengType(int val);

	virtual void mousePressEvent(QMouseEvent *evt);
	virtual void mouseReleaseEvent(QMouseEvent *evt);
	virtual void keyPressEvent(QKeyEvent *evt) {}
	virtual void keyReleaseEvent(QKeyEvent *evt) {}
	virtual void mouseMoveEvent(QMouseEvent *evt) {}
	virtual void wheelEvent(QWheelEvent *evt) {}
	virtual void mouseDoubleClickEvent(QMouseEvent *evt){}

private:
	FengBaoChaoPlugin(const FengBaoChaoPlugin&);  // disable copy construction
	FengBaoChao* fbc_;
	ManualNode* mn_;
    void OnSpeedBtn(bool val);
	void OnTempBtn(bool val);
	void OnPressBtn(bool val);
	void OnCancelBtn(bool val);
	void OnCutBtn(bool val);
	void OnRecoverBtn(bool val);

	bool node_loaded_;
	bool on_cut_;

	int mouse_x;
	int mouse_y;
	bool lbtn;
	bool rbtn;

private:
    RequestDispatcher<ToggleFunc> toggle_disp_;
};

END_PLUGIN_NAMESPACE

#endif // _QUICK_PLUGIN_H
