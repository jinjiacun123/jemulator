/*
    Copyright 2022-2024 Hydr8gon

    This file is part of rokuyon.

    rokuyon is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    rokuyon is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with rokuyon. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ry_frame.h"
#include "ry_canvas.h"
#include "../core.h"
#include "../pif.h"
#include "../settings.h"

enum FrameEvent
{
    LOAD_ROM = 1,
    QUIT
};

wxBEGIN_EVENT_TABLE(ryFrame, wxFrame)
EVT_MENU(QUIT, ryFrame::quit)
EVT_DROP_FILES(ryFrame::dropFiles)
EVT_CLOSE(ryFrame::close)
wxEND_EVENT_TABLE()

ryFrame::ryFrame(std::string path): wxFrame(nullptr, wxID_ANY, "rokuyon")
{
#if 1 //[by jim] setting window property
    // Set up and show the window
    DragAcceptFiles(true);
#if 1
    SetClientSize(MIN_SIZE);
	printf("set SetClientSize sizer\n");
    //SetMinClientSize(MIN_SIZE);
	printf("set SetMinClientSize sizer\n");
#else
	SetClientSize(wxSize(400,400));
    SetMinClientSize(wxSize(400,400));
#endif
    Centre();
    Show(true);
#endif	

#if 1 //[by jim] draw frame
    // Set up a canvas for drawing the framebuffer
    canvas = new ryCanvas(this);
#if 1
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
#else
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
#endif
    sizer->Add(canvas, 1, wxEXPAND);
	printf("set wxBoxSizer sizer\n");
    //SetSizer(sizer);
#endif	

#if 1 //[by jim] joystick
    // Prepare a joystick if one is connected
    timer = nullptr;
#endif

#if 1 //[by jim] call bootRom
    // Boot a ROM right away if a filename was given through the command line
    if (path != "")
        bootRom(path);
#endif
}

void ryFrame::bootRom(std::string path)//[by jim]<------------------------
{
    // Remember the path so the ROM can be restarted
    lastPath = path;

    // Try to boot the specified ROM, and display an error if failed
    if (!Core::bootRom(path))  //[by jim]<------------------------
    {
        wxMessageDialog(this, "Make sure the ROM file is accessible and try again.",
            "Error Loading ROM", wxICON_NONE).ShowModal();
        return;
    }

    // Reset the system menu
    paused = false;
}

void ryFrame::Refresh()
{
    wxFrame::Refresh();

    // Override the refresh function to also update the FPS counter
    wxString label = "rokuyon";
    if (Core::running)
        label += wxString::Format(" - %d FPS", Core::fps);
    SetLabel(label);
}

void ryFrame::loadRom(wxCommandEvent &event)//[by jim]:<----------------------------
{
    // Show the file browser
    wxFileDialog romSelect(this, "Select ROM File", "", "", "N64 ROM files (*.z64)|*.z64", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    // Boot a ROM if a file was selected
    if (romSelect.ShowModal() != wxID_CANCEL)
        bootRom((const char*)romSelect.GetPath().mb_str(wxConvUTF8));
}

void ryFrame::quit(wxCommandEvent &event)
{
    // Close the program
    Close(true);
}

void ryFrame::dropFiles(wxDropFilesEvent &event)
{
    // Boot a ROM if a single file is dropped onto the frame
    if (event.GetNumberOfFiles() == 1)
    {
        wxString path = event.GetFiles()[0];
        if (wxFileExists(path))
            bootRom((const char*)path.mb_str(wxConvUTF8));
    }
}

void ryFrame::close(wxCloseEvent &event)
{
    // Stop emulation before exiting
    Core::stop();
    canvas->finish();
    event.Skip(true);
}
