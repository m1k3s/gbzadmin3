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
#include <iostream>
#include "gbzadmin.h"
#include "config.h"

const char *xml_interface = "gbzadmin3.xml";

int main(int argc, char *argv[])
{
	Gnome::Conf::init();
	Gtk::Main admin(argc, argv);
	Glib::ustring xml_file;
	
	// find the glade file. If the program is launched from the devel
	// directory, use the local glade file. Otherwise if the program 
	// is launched with an absolute path, use the glade file from
	// the PACKAGE_DATA_DIR / PACKAGE path.
	Glib::ustring prog_name = Glib::find_program_in_path(argv[0]);

	if (!prog_name.empty()) {
		static const Glib::ustring dir(G_DIR_SEPARATOR_S "src" G_DIR_SEPARATOR_S "gbzadmin3");

		if (!Glib::path_is_absolute(prog_name)) {
			Glib::ustring cur_dir = Glib::get_current_dir();
			
			Glib::ustring prog_absolute;
			if (Glib::str_has_prefix(prog_name, "." G_DIR_SEPARATOR_S)) {
				Glib::ustring tmp((prog_name.c_str())+2);
				prog_absolute = Glib::build_filename(cur_dir, tmp);
			} else {
				prog_absolute = Glib::build_filename(cur_dir, prog_name);
			}
			prog_name = prog_absolute;
		}

		if (Glib::str_has_suffix(prog_name, dir)) {
			gchar *suffix = g_strrstr (prog_name.c_str(), dir.c_str());
			if (suffix) {
				*suffix = 0;
				xml_file = Glib::build_filename(prog_name, xml_interface);
			}
		}
		
		if (!Glib::file_test(xml_file, Glib::FILE_TEST_EXISTS)) {
			xml_file.clear();
		}
	}

	if (xml_file.empty()) {
		std::vector<Glib::ustring> path;
		path.push_back(PACKAGE_DATA_DIR);
		path.push_back(PACKAGE);
		path.push_back(xml_interface);
		xml_file = Glib::build_filename(path);
	} else {
		std::cout << "Using local XML file: " << xml_file << std::endl;
	}

	// Load the xml file and instantiate its objects:
	Glib::RefPtr<Gtk::Builder> refBuilder; 

	try	{
		refBuilder = Gtk::Builder::create_from_file(xml_file);
	}
	catch (const Glib::FileError& ex)	{
		std::cerr << "\n *** " << ex.what() << std::endl;
		return 1;
	}
	// instantiate the main class 'gbzadmin'
	Gtk::Window *app;
	gbzadmin gbzadmin;
	
	// initialize gbzadmin
	if (!(app = gbzadmin.init_gbzadmin(refBuilder))) {
		std::cout << "Failed to initialize gbzadmin" << std::endl;
		return 1;
	}
	// let's go...
	admin.run(*app);
	
	return 0;
}

