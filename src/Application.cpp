#include "Application.hpp"

wxIMPLEMENT_APP(Application); // int main()

bool Application::OnInit()
{
	Frame* frame = new Frame("Enterprise Portal Services Controller");
	frame->Show();
	return true;
}

Frame::Frame(const std::string&& title)
	: wxFrame(nullptr, wxID_ANY, title, wxPoint(wxDefaultPosition),
		wxSize(wxDefaultSize), wxDEFAULT_FRAME_STYLE),
	  m_configuration{},
	  m_portalServerControl{
		  std::make_unique<PortalServerControls>(m_configuration) },
	  m_serverCommandMethod{ m_configuration.getCommandMethod() }
{
	SetSize(m_configuration.getWindowSizeX(),
			m_configuration.getWindowSizeY());

	wxPanel* panel = new wxPanel(
		this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	panel->Bind(wxEVT_CHAR_HOOK, &Frame::keyboardShortcuts, this);

	// BEGIN MENU BAR
	wxMenuBar* menuBar = new wxMenuBar();

	wxMenu* fileMenu = new wxMenu();
		
		auto credentials = fileMenu->Append(
			MenuID::changeCredentials,
			"Change Enterprise Portal Credentials");
		credentials->SetBitmap(wxArtProvider::GetBitmap(
			wxART_INFORMATION, wxART_MENU));

			wxMenu* changeMethodSubmenu = new wxMenu();
			wxMenuItem* changeMethod = fileMenu->AppendSubMenu(
				changeMethodSubmenu, "Change Server Command Method");
			changeMethod->SetBitmap(wxArtProvider::GetBitmap(
				wxART_WARNING, wxART_MENU));
			changeMethod->SetSubMenu(changeMethodSubmenu);

				m_menuModeSequential = changeMethodSubmenu->Append(
					MenuID::setMethodSequential,
					"Sequential Commands", "", wxITEM_CHECK);
				m_menuModeSequential->SetBitmaps(wxArtProvider::GetBitmap(
					wxART_TICK_MARK, wxART_MENU), wxNullBitmap);

				m_menuModeBatch = changeMethodSubmenu->Append(
					MenuID::setMethodBatch,
					"Single Batch Command", "", wxITEM_CHECK);
				m_menuModeBatch->SetBitmaps(wxArtProvider::GetBitmap(
					wxART_TICK_MARK, wxART_MENU), wxNullBitmap);

				if (m_serverCommandMethod
					== Constants::Commands::sequentialMode)
				{
					m_menuModeSequential->Check(true);
					m_menuModeBatch->Check(false);
				}
				else
				{
					m_menuModeSequential->Check(false);
					m_menuModeBatch->Check(true);
				}

		fileMenu->AppendSeparator();

		wxMenuItem* quit = fileMenu->Append(
			MenuID::quit,
			"Quit");
		quit->SetBitmap(wxArtProvider::GetBitmap(
			wxART_QUIT, wxART_MENU));

	wxMenu* statusMenu = new wxMenu();

		wxMenuItem* find = statusMenu->Append(
			MenuID::find,
			"Find");
		find->SetBitmap(wxArtProvider::GetBitmap(
			wxART_FIND, wxART_MENU));

		wxMenuItem* copySelection = statusMenu->Append(
			MenuID::copySelection,
			"Copy Selection");
		copySelection->SetBitmap(wxArtProvider::GetBitmap(
			wxART_COPY, wxART_MENU));

		wxMenuItem* copyAll = statusMenu->Append(
			MenuID::copyAll,
			"Copy Server Status");
		copyAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_COPY, wxART_MENU));

		statusMenu->AppendSeparator();

		wxMenuItem* refreshStatus = statusMenu->Append(
			MenuID::refresh,
			"Refresh Server Status");
		refreshStatus->SetBitmap(wxArtProvider::GetBitmap(
			wxART_REFRESH, wxART_MENU));

		statusMenu->AppendSeparator();

		wxMenuItem* showAll = statusMenu->Append(
			MenuID::showAll,
			"Show All Services");
		showAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_REPORT_VIEW, wxART_MENU));

		wxMenuItem* showStarted = statusMenu->Append(
			MenuID::showStarted,
			"Show Only Started Services");
		showStarted->SetBitmap(wxArtProvider::GetBitmap(
			wxART_GO_FORWARD, wxART_MENU));

		wxMenuItem* showStopped = statusMenu->Append(
			MenuID::showStopped,
			"Show Only Stopped Services");
		showStopped->SetBitmap(wxArtProvider::GetBitmap(
			wxART_STOP, wxART_MENU));

	wxMenu* controlsMenu = new wxMenu();

		wxMenu* exportJsonSubMenu = new wxMenu();
		wxMenuItem* exportJson = controlsMenu->AppendSubMenu(
			exportJsonSubMenu, "Export Batch Command as JSON");
		exportJson->SetBitmap(wxArtProvider::GetBitmap(
			wxART_GO_DIR_UP, wxART_MENU));
		exportJson->SetSubMenu(exportJsonSubMenu);

		controlsMenu->AppendSeparator();

			wxMenuItem* exportStart = exportJsonSubMenu->Append(
				MenuID::exportStart,
				"Export Start All Command");
			exportStart->SetBitmap(wxArtProvider::GetBitmap(
				wxART_GO_FORWARD, wxART_MENU));

			wxMenuItem* exportStop = exportJsonSubMenu->Append(
				MenuID::exportStop,
				"Export Stop All Command");
			exportStop->SetBitmap(wxArtProvider::GetBitmap(
				wxART_STOP, wxART_MENU));

		wxMenuItem* startAll = controlsMenu->Append(
			MenuID::start,
			"Start All Map Services");
		startAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_GO_FORWARD, wxART_MENU));

		controlsMenu->AppendSeparator();

		wxMenuItem* stopAll = controlsMenu->Append(
			MenuID::stop,
			"Stop All Map Services");
		stopAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_STOP, wxART_MENU));

	menuBar->Append(fileMenu, "File");
	menuBar->Append(statusMenu, "Server Status");
	menuBar->Append(controlsMenu, "Server Controls");

	SetMenuBar(menuBar);
	menuBar->Bind(wxEVT_MENU, &Frame::menuAction, this);

	// BEGIN TEXT BOX ITEMS
	m_statusText = new CustomRichTextCtrl(panel, wxRE_READONLY);
	wxBoxSizer* textSizer = new wxBoxSizer(wxHORIZONTAL);
	textSizer->Add(m_statusText, 1, wxEXPAND | wxALL);
	panel->SetSizerAndFit(textSizer);

	displayWelcomeMessage();

	wxMenu* contextStatusMenu = new wxMenu();

		auto contextfind = contextStatusMenu->Append(
			MenuID::find,
			"Find");
		contextfind->SetBitmap(wxArtProvider::GetBitmap(
			wxART_FIND, wxART_MENU));

		auto contextcopySelection = contextStatusMenu->Append(
			MenuID::copySelection,
			"Copy Selection");
		contextcopySelection->SetBitmap(wxArtProvider::GetBitmap(
			wxART_COPY, wxART_MENU));

		auto contextcopyAll = contextStatusMenu->Append(
			MenuID::copyAll,
			"Copy Complete Server Status");
		contextcopyAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_COPY, wxART_MENU));

		contextStatusMenu->AppendSeparator();

		auto contextRefresh = contextStatusMenu->Append(
			MenuID::refresh,
			"Refresh Server Status");
		contextRefresh->SetBitmap(wxArtProvider::GetBitmap(
			wxART_REFRESH, wxART_MENU));

		contextStatusMenu->AppendSeparator();

		auto contextShowAll = contextStatusMenu->Append(
			MenuID::showAll,
			"Show All Services");
		contextShowAll->SetBitmap(wxArtProvider::GetBitmap(
			wxART_EXECUTABLE_FILE, wxART_MENU));

		auto contextShowStarted = contextStatusMenu->Append(
			MenuID::showStarted,
			"Show Only Started Services");
		contextShowStarted->SetBitmap(wxArtProvider::GetBitmap(
			wxART_TICK_MARK, wxART_MENU));

		auto contextShowStopped = contextStatusMenu->Append(
			MenuID::showStopped,
			"Show Only Stopped Services");
		contextShowStopped->SetBitmap(wxArtProvider::GetBitmap(
			wxART_CROSS_MARK, wxART_MENU));

	m_statusText->SetContextMenu(contextStatusMenu);
	contextStatusMenu->Bind(wxEVT_MENU, &Frame::menuAction, this);

	if (!m_configuration.credentialsAquired())
	{
		enterCredentials();
	}
}

Frame::~Frame()
{
	wxSize windowSize = GetSize();
	m_configuration.updateConfigSettings(
		windowSize.x, windowSize.y, m_serverCommandMethod);
}

void Frame::areYouSure(const std::string_view command)
{
	wxMessageDialog* youSure = new wxMessageDialog(
		this,
		(command == Constants::Commands::START
				? "START all services on the server?"
				: "STOP all services on the server?")
		+ Constants::Messages::durationWarning,
		"Are You Sure?",
		wxYES_NO);

	if (youSure->ShowModal() == wxID_YES)
	{
		if (m_serverCommandMethod == Constants::Commands::sequentialMode)
		{
			sendSequentialCommand(command);
		}
		else
		{
			sendBatchCommand(command);
		}
	}
}

void Frame::copyServerStatus()
{
	m_statusText->SelectAll();
	m_statusText->Copy();
	m_statusText->SelectNone();
}

void Frame::displayStatusAll()
{
	m_statusText->Clear();

	if (m_statusText->IsSelectionBold())
	{
		m_statusText->ApplyBoldToSelection();
	}
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->SetBackgroundColour(*wxLIGHT_GREY);
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 0, 0, 255 });
	m_statusText->WriteText(
		"Enterprise Portal: " + m_configuration.getPortal());
	m_statusText->EndTextColour();
	m_statusText->Newline();
	m_statusText->BeginTextColour({ 21, 67, 96 });
	m_statusText->WriteText(m_portalServerControl->getTimeStamp());
	m_statusText->Newline();
	m_statusText->WriteText(
		"Total Services: " + m_portalServerControl->getCountTotalAsString());
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStartedAsString() + " - Started.");
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStoppedAsString() + " - Stopped.");
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	m_statusText->Newline();
	m_statusText->Newline();
	if (!m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_CENTER);
	}
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 23, 32, 42, });
	m_statusText->WriteText("*** Displaying All Services ***");
	m_statusText->Newline();
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->Newline();

	for (const auto& [folder, services]
		: m_portalServerControl->m_serviceInformation)
	{
		m_statusText->BeginBold();
		m_statusText->BeginTextColour({ 23, 32, 42 });
		m_statusText->WriteText(folder + ":");
		m_statusText->Newline();
		m_statusText->EndTextColour();
		m_statusText->EndBold();
		for (const auto& [service, status] : services)
		{
			if (status == "STARTED") // Green Text if STARTED.
			{
				m_statusText->BeginTextColour({ 0, 139, 0 });
				m_statusText->WriteText("\t" + service + " - " + status);
				m_statusText->EndTextColour();
			}
			else // Red Text if STOPPED.
			{
				m_statusText->BeginTextColour({ 255, 0, 0 });
				m_statusText->WriteText("\t" + service + " - " + status);
				m_statusText->EndTextColour();
			}
			m_statusText->Newline();
		}
		m_statusText->Newline();
	}
}

void Frame::displayStatusStarted()
{
	m_statusText->Clear();

	if (m_statusText->IsSelectionBold())
	{
		m_statusText->ApplyBoldToSelection();
	}
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->SetBackgroundColour(*wxLIGHT_GREY);
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 0, 0, 255 });
	m_statusText->WriteText(
		"Enterprise Portal: " + m_configuration.getPortal());
	m_statusText->EndTextColour();
	m_statusText->Newline();
	m_statusText->BeginTextColour({ 21, 67, 96 });
	m_statusText->WriteText(m_portalServerControl->getTimeStamp());
	m_statusText->Newline();
	m_statusText->WriteText(
		"Total Services: " + m_portalServerControl->getCountTotalAsString());
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStartedAsString() + " - Started.");
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStoppedAsString() + " - Stopped.");
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	m_statusText->Newline();
	m_statusText->Newline();
	if (!m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_CENTER);
	}
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 23, 32, 42, });
	m_statusText->WriteText("*** Displaying Started Services ***");
	m_statusText->Newline();
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->Newline();

	for (const auto& [folder, services]
		: m_portalServerControl->m_serviceInformation)
	{
		m_statusText->BeginBold();
		m_statusText->BeginTextColour({ 23, 32, 42 });
		m_statusText->WriteText(folder + ":");
		m_statusText->Newline();
		m_statusText->EndTextColour();
		m_statusText->EndBold();
		for (const auto& [service, status] : services)
		{
			if (status == "STARTED") // Green Text if STARTED.
			{
				m_statusText->BeginTextColour({ 0, 139, 0 });
				m_statusText->WriteText("\t" + service + " - " + status);
				m_statusText->EndTextColour();
				m_statusText->Newline();
			}
		}
		m_statusText->Newline();
	}
}

void Frame::displayStatusStopped()
{
	m_statusText->Clear();

	if (m_statusText->IsSelectionBold())
	{
		m_statusText->ApplyBoldToSelection();
	}
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->SetBackgroundColour(*wxLIGHT_GREY);
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 0, 0, 255 });
	m_statusText->WriteText(
		"Enterprise Portal: " + m_configuration.getPortal());
	m_statusText->EndTextColour();
	m_statusText->Newline();
	m_statusText->BeginTextColour({ 21, 67, 96 });
	m_statusText->WriteText(m_portalServerControl->getTimeStamp());
	m_statusText->Newline();
	m_statusText->WriteText(
		"Total Services: " + m_portalServerControl->getCountTotalAsString());
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStartedAsString() + " - Started.");
	m_statusText->Newline();
	m_statusText->WriteText(
		m_portalServerControl->getCountStoppedAsString() + " - Stopped.");
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	m_statusText->Newline();
	m_statusText->Newline();
	if (!m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_CENTER);
	}
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 23, 32, 42, });
	m_statusText->WriteText("*** Displaying Stopped Services ***");
	m_statusText->Newline();
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->Newline();

	for (const auto& [folder, services]
		: m_portalServerControl->m_serviceInformation)
	{
		m_statusText->BeginBold();
		m_statusText->BeginTextColour({ 23, 32, 42 });
		m_statusText->WriteText(folder + ":");
		m_statusText->EndTextColour();
		m_statusText->EndBold();
		m_statusText->Newline();
		for (const auto& [service, status] : services)
		{
			if (status == "STOPPED") // Red Text if STOPPED.
			{
				m_statusText->BeginTextColour({ 255, 0, 0 });
				m_statusText->WriteText("\t" + service + " - " + status);
				m_statusText->EndTextColour();
				m_statusText->Newline();
			}
		}
		m_statusText->Newline();
	}
}

void Frame::displayWelcomeMessage()
{
	m_statusText->Clear();

	if (m_statusText->IsSelectionBold())
	{
		m_statusText->ApplyBoldToSelection();
	}
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->SetBackgroundColour(*wxLIGHT_GREY);
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 0, 0, 255 });
	m_statusText->WriteText("Enterprise Portal: " + m_configuration.getPortal());
	m_statusText->EndBold();
	m_statusText->EndTextColour();
	m_statusText->Newline();
	m_statusText->Newline();
	m_statusText->Newline();
	if (!m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_CENTER);
	}
	m_statusText->BeginBold();
	m_statusText->BeginTextColour({ 21, 67, 96 });
	m_statusText->WriteText(m_portalServerControl->getTimeStamp());
	m_statusText->Newline();
	m_statusText->EndTextColour();
	m_statusText->EndBold();
	if (m_statusText->IsSelectionAligned(wxTEXT_ALIGNMENT_CENTER))
	{
		m_statusText->ApplyAlignmentToSelection(wxTEXT_ALIGNMENT_LEFT);
	}
	m_statusText->Newline();
}

void Frame::enterCredentials()
{
	EnterCredentials* credentialsWindow = 
		new EnterCredentials(this, m_configuration);
	credentialsWindow->ShowModal();
}

void Frame::exportJson(MenuID mode)
{
	refreshStatus();

	std::string jsonString{ m_portalServerControl->generateJson(
		mode == MenuID::exportStart
		? Constants::Commands::START
		: Constants::Commands::STOP) };

	auto isUsable = [](const std::string& jsonString) -> bool
	{ return jsonString.find("type") != std::string::npos; };

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(isUsable(jsonString)
			? jsonString : "Services are already at requested state."));
		wxTheClipboard->Close();

		wxMessageDialog* copied = new wxMessageDialog(this,
			"JSON loaded into clipboard.",
			"Copied",
			wxOK | wxICON_INFORMATION);
		copied->ShowModal();
	}
	else
	{
		wxMessageDialog* copyError = new wxMessageDialog(this,
			"Unable to open clipboard.",
			"Error",
			wxOK | wxICON_INFORMATION);
		copyError->ShowModal();
	}
}


void Frame::keyboardShortcuts(wxKeyEvent& event)
{
	if (m_portalServerControl->credentialsAreValid())
	{
		// F3 or Ctrl + F
		if (event.GetKeyCode() == WXK_F3 ||
			(event.GetModifiers() == wxMOD_CONTROL
				&& event.GetKeyCode() == 'F'))
		{
			showFindTextPrompt();
		}
		// F5
		else if (event.GetKeyCode() == WXK_F5)
		{
			refreshStatus();
		}
		// Ctrl + A
		else if (event.GetModifiers() == wxMOD_CONTROL
			&& event.GetKeyCode() == 'A')
		{
			m_statusText->SelectAll();
		}
		// Ctrl + C
		else if (event.GetModifiers() == wxMOD_CONTROL
			&& event.GetKeyCode() == 'C')
		{
			m_statusText->Copy();
		}
	}
	else
	{
		wxMessageDialog* noCred = new wxMessageDialog(this,
			Constants::Messages::credentialsRequired,
			"Credentials Required", wxOK | wxICON_INFORMATION);
		noCred->ShowModal();
	}
}

void Frame::menuAction(wxCommandEvent& event)
{
	if (m_portalServerControl->credentialsAreValid())
	{
		switch (event.GetId())
		{
		case MenuID::changeCredentials:
		{
			enterCredentials();
			updateCredentials();
			displayWelcomeMessage();
			break;
		}

		case MenuID::quit:
			Close();
			break;

		case MenuID::setMethodSequential:
		{
			m_serverCommandMethod = Constants::Commands::sequentialMode;
			m_menuModeSequential->Check(true);
			m_menuModeBatch->Check(false);

			wxMessageDialog* sequentialSet = new wxMessageDialog(this,
				"Server command mode set to 'Sequential' mode.",
				"Command Mode Changed", wxOK | wxICON_INFORMATION);
			sequentialSet->ShowModal();
			break;
		}

		case MenuID::setMethodBatch:
		{
			m_serverCommandMethod = Constants::Commands::batchMode;
			m_menuModeBatch->Check(true);
			m_menuModeSequential->Check(false);

			wxMessageDialog* batchSet = new wxMessageDialog(this,
				"Server command mode set to 'Single Batch' mode.",
				"Command Mode Changed", wxOK | wxICON_INFORMATION);
			batchSet->ShowModal();
			break;
		}

		case MenuID::find:
			showFindTextPrompt();
			break;

		case MenuID::copyAll:
			copyServerStatus();
			break;

		case MenuID::copySelection:
			m_statusText->Copy();
			break;

		case MenuID::refresh:
			refreshStatus();
			break;

		case MenuID::showAll:
			if (!m_portalServerControl->isStatusValid())
			{
				refreshStatus();
			}
			displayStatusAll();
			break;

		case MenuID::showStarted:
			if (!m_portalServerControl->isStatusValid())
			{
				refreshStatus();
			}
			displayStatusStarted();
			break;

		case MenuID::showStopped:
			if (!m_portalServerControl->isStatusValid())
			{
				refreshStatus();
			}
			displayStatusStopped();
			break;

		case MenuID::exportStart:
			exportJson(MenuID::exportStart);
			break;

		case MenuID::exportStop:
			exportJson(MenuID::exportStop);
			break;

		case MenuID::start:
			areYouSure(Constants::Commands::START);
			break;
		case MenuID::stop:
			areYouSure(Constants::Commands::STOP);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (event.GetId())
		{
		case MenuID::quit:
		{
			Close();
			break;
		}
		case MenuID::changeCredentials:
		{
			enterCredentials();
			updateCredentials();
			displayWelcomeMessage();
			break;
		}
		default:
			wxMessageDialog* noCred = new wxMessageDialog(this,
				Constants::Messages::credentialsRequired,
				"Credentials Required", wxOK | wxICON_INFORMATION);
			noCred->ShowModal();
			break;
		}
	}
}

void Frame::refreshStatus()
{
	threadState state(0,
		m_portalServerControl->getCountTotal(),
		Constants::Messages::gathering,
		true);

	wxGenericProgressDialog progress(
		Constants::Messages::gathering,
		state.message,
		state.progressMax,
		this,
		wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

	std::thread update([this, &state]()
		{ m_portalServerControl->updateStatus(state); });
	update.detach();
	progress.Show();
	
	while (state.working)
	{
		progress.Update(state.progressValue, state.message);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	displayStatusAll();
}

void Frame::sendBatchCommand(const std::string_view command)
{
	std::chrono::time_point start{ std::chrono::system_clock::now() };

	std::promise<nlohmann::json> promise;
	std::shared_future<nlohmann::json> future = promise.get_future();
	std::thread sendServerCommand(
		[this, promise = std::move(promise), command]() mutable
		{
			m_portalServerControl->sendBatchServerCommand(
				std::move(promise), command);
		});
	sendServerCommand.detach();

	wxGenericProgressDialog progress(
		command == Constants::Commands::START
		? "Sending 'Start All' command to server."
		: "Sending 'Stop All' command to server.",
		Constants::Messages::waitCommand,
		10,
		this,
		wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

	progress.Show();

	while (future.wait_for(std::chrono::milliseconds(50))
		!= std::future_status::ready)
	{
		progress.Pulse();
	}
	progress.Hide();

	nlohmann::json result = future.get();
	std::chrono::time_point finish{ std::chrono::system_clock::now() };
	auto elapsedTime =
		std::chrono::duration_cast<std::chrono::seconds>(finish - start);
	displayStatusAll();

	wxMessageDialog* done = new wxMessageDialog(this,
		"Operation completed in "
		+ std::to_string(elapsedTime.count()) + " seconds.\n\n"
		+ "Server Response: " + result["status"].get<std::string>(),
		"Operation Complete", wxOK | wxICON_INFORMATION);
	done->ShowModal();
}

void Frame::sendSequentialCommand(const std::string_view command)
{
	std::chrono::time_point start{ std::chrono::system_clock::now() };

	constexpr int numberOfStepsToComplete{ 4 };
	threadState state(0,
		m_portalServerControl->getCountTotal() * numberOfStepsToComplete,
		Constants::Messages::gathering,
		true);

	wxGenericProgressDialog progress(
		command == Constants::Commands::START
		? "Sending 'Start All' command to server."
		: "Sending 'Stop All' command to server.",
		"Server command mode:\nSequential",
		state.progressMax,
		this,
		wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME);

	std::promise<nlohmann::json> promise;
	std::shared_future<nlohmann::json> future = promise.get_future();
	std::thread sendServerCommand(
		[this, promise = std::move(promise), command, &state]() mutable
		{
			m_portalServerControl->sendSequentialServerCommands(
				std::move(promise), command, state);
		});

	sendServerCommand.detach();
	progress.Show();

	while (state.working)
	{
		progress.Update(state.progressValue, state.message);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	nlohmann::json result = future.get();

	std::chrono::time_point finish{ std::chrono::system_clock::now() };
	auto elapsedTime =
		std::chrono::duration_cast<std::chrono::seconds>(finish - start);
	displayStatusAll();

	wxMessageDialog* done = new wxMessageDialog(this,
		"Operation completed in "
		+ std::to_string(elapsedTime.count()) + " seconds.\n\n"
		+ "Server Response: " + result["status"].get<std::string>(),
		"Operation Complete", wxOK | wxICON_INFORMATION);
	done->ShowModal();
}

void Frame::showFindTextPrompt()
{
	wxFindReplaceDialog* findTextWindow = new wxFindReplaceDialog(
		this, &m_findOptions, "Find");

	// First click of "Find Next".
	findTextWindow->Bind(wxEVT_FIND,
		[this, findTextWindow](wxFindDialogEvent& event)
		{
			m_statusText->findText(findTextWindow, event);
		});

	// Additional clicks of "Find Next".
	findTextWindow->Bind(wxEVT_FIND_NEXT,
		[this, findTextWindow](wxFindDialogEvent& event)
		{
			m_statusText->findText(findTextWindow, event);
		});

	findTextWindow->Show();
}

void Frame::updateCredentials()
{
	std::promise<void> promise;
	std::future<void> future = promise.get_future();

	wxGenericProgressDialog progress(
		"Updating credentials, please wait...",
		"Updating Credentials",
		10,
		this,
		wxPD_APP_MODAL | wxPD_AUTO_HIDE);
	progress.Show();

	std::thread update([this, promise = std::move(promise)]() mutable
		{
			m_portalServerControl.reset(new PortalServerControls(m_configuration));
			promise.set_value_at_thread_exit();
		});
	update.detach();

	while (future.wait_for(std::chrono::milliseconds(50))
		!= std::future_status::ready)
	{
		progress.Pulse();
	}
	progress.Hide();
}