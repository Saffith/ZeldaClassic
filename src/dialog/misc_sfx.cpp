#include "misc_sfx.h"
#include "info.h"
#include "alert.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"
#include "gui/use_size.h"
using GUI::sized;

static const GUI::ListData miscSFXList
{
	{ "Grass Cut:", sfxBUSHGRASS, "Played when bush/grass/etc is cut, and does not have a custom SFX set." },
	{ "'Switch With Player':", sfxSWITCHED, "Played when an enemy's 'Switch w/ Player' defense is triggered, or a switchhook with 0 for 'Switch Sound' switches." },
	{ "Low Health Beep:", sfxLOWHEART, "Played when the player's health is low." }
};

MiscSFXDialog::MiscSFXDialog(byte* vals, size_t vals_per_tab, std::function<void(int32_t*)> setVals):
	setVals(setVals), vals_per_tab(vals_per_tab), sfx_list(GUI::ListData::sfxnames())
{
	for(auto q = 0; q < sfxMAX; ++q)
	{
		local_sfx[q] = vals[q];
	}
}

std::shared_ptr<GUI::Widget> MiscSFXDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	return Window(
		title = "Misc Sounds",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			maxwidth = sized(308_px, 800_px),
			DDPanel(
				padding = 2_spx,
				values = local_sfx,
				count = vals_per_tab,
				ddlist = sfx_list,
				data = miscSFXList
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
}

bool MiscSFXDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			setVals(local_sfx);
		[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
	}
}