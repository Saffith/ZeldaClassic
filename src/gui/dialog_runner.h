#ifndef ZC_GUI_DIALOGRUNNER_H
#define ZC_GUI_DIALOGRUNNER_H

#include "dialog.h"
#include "dialog_message.h"
#include "dialog_ref.h"
#include "helper.h"
#include "widget.h"
#include "../signal.h"
#include <memory>
#include <type_traits>

namespace GUI
{

class DialogRunner
{
public:
	template<typename T>
	void run(T& dlg)
	{
		sendMessage =
			[this, &dlg](int msg, MessageArg arg, std::shared_ptr<Widget> snd)
			{
				DialogMessage<typename T::message> dm;
				dm.message = static_cast<typename T::message>(msg);
				dm.argument = arg;
				dm.sender = snd;
				this->done = this->done || dlg.handleMessage(dm);
			};

		runInner(dlg.view());
	}

	/* Add a DIALOG and connect it to its owner.
	 * This should always be called as
	 * runner.push(shared_from_this(), DIALOG { ... });
	 * Returns a DialogRef that can be used as a reference to the
	 * newly added DIALOG.
	 */
	DialogRef push(std::shared_ptr<Widget> owner, DIALOG dlg);

	/* Returns the current size of the DIALOG array. */
	inline size_t size() const
	{
		return alDialog.size();
	}

	/* Returns a raw pointer to the DIALOG array for widgets that need it.
	 * This must not be called until the array is fully constructed.
	 * This can be done using the dialogConstructed signal.
	 */
	inline DIALOG* getDialogArray()
	{
		return alDialog.data();
	}

	bool isConstructed()
	{
		return realized;
	}

	bool allowDraw()
	{
		return running;
	}
	
	void pendDraw()
	{
		redrawPending = true;
	}

	/* A signal emitted when construction of the DIALOG array is finished.
	 * Shouldn't really be public, but that can be dealt with later.
	 */
	Signal dialogConstructed;

private:
	std::function<void(int, MessageArg, std::shared_ptr<Widget>)> sendMessage;
	std::vector<DIALOG> alDialog;
	std::vector<std::shared_ptr<Widget>> widgets;
	int focused;
	bool redrawPending, done, realized, running;

	DialogRunner();

	/* Sets up the DIALOG array for a dialog so that it can be run. */
	void realize(std::shared_ptr<Widget> root);

	void runInner(std::shared_ptr<Widget> root);

	friend class DialogRef;
	friend int dialog_proc(int msg, DIALOG *d, int c);
	template<typename T> friend void showDialog(T& dlg);
};

// Separate from DialogRunner due to type resolution limitations.
template<typename T>
inline void showDialog(T& dlg)
{
	auto dr=DialogRunner();
	dr.run(dlg);
}

}

#endif
