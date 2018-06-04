// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
//  
// Code based on BZFlag-2.0.x
// Portions Copyright (c) 1993 - 2009 Tim Riker
// 
//  =====GPL=============================================================
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 dated June, 1991.
// 
//  This program is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program;  if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
//  =====================================================================

#include <gtkmm.h>
#include <gstreamermm.h>


Sound::Sound()
{
	pipeline = Gst::Pipeline::create("note");
	source = Gst::ElementFactory::create_element("audiotestsrc", "source");
	sink = Gst::ElementFactory::create_element("autoaudiosink", "output");
	pipeline->add(m_source);
	pipeline->add(m_sink);
	source->link(m_sink);
}

void Sound::start_playing (double frequency)
{
	source->set_property("freq", frequency);
	pipeline->set_state(Gst::STATE_PLAYING);

	// stop it after 200ms
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Sound::stop_playing), 200);
}

bool Sound::stop_playing()
{
	pipeline->set_state(Gst::STATE_NULL);
	return false;
}

static void on_button_clicked(double frequency, Sound* sound)
{
	sound->start_playing (frequency);
}
                   
//int main(int argc, char *argv[])
//{
//	Gtk::Main kit(argc, argv);
//	Gst::init (argc, argv);
//	
//	// Load the Glade file and instantiate its widgets:
//	Glib::RefPtr<Gtk::Builder> builder;
//	try
//	{
//		builder = Gtk::Builder::create_from_file(UI_FILE);
//	}
//	catch (const Glib::FileError & ex)
//	{
//		std::cerr << ex.what() << std::endl;
//		return 1;
//	}
//	Gtk::Window* main_win = 0;
//	builder->get_widget("main_window", main_win);

//	Sound sound;
//	Gtk::Button* button = 0;
//	
//	builder->get_widget("button_E", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 369.23, &sound));
//	builder->get_widget("button_A", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 440, &sound));
//	builder->get_widget("button_D", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 587.33, &sound));
//	builder->get_widget("button_G", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 783.99, &sound));
//	builder->get_widget("button_B", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 987.77, &sound));
//	builder->get_widget("button_e", button);
//	button->signal_clicked().connect (sigc::bind<double, Sound*>(sigc::ptr_fun(&on_button_clicked), 1318.5, &sound));
//	
//	kit.run(*main_win);

//	return 0;
//}


