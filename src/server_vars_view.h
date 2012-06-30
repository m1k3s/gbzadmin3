#ifndef _server_vars_h_
#define _server_vars_h_
// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2009 Michael Sheppard
//  
// Code based on BZFlag-2.0.x
// Portions Copyright (c) 1993 - 2005 Tim Riker
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
#include "default_vars.h"

#define VARIABLE   "purple"
#define VALUE      "cyan3"
#define DEFAULT    "DarkOrange"
#define CHANGED    "yellow"
#define BKGRND     "black"
#define FONT       "Monospace 9"

class Variable
{
public:
	Variable();
	Variable(Glib::ustring var, Glib::ustring val);
	~Variable() {}
	
	Glib::ustring _var;
	Glib::ustring _val;
	Glib::ustring _def;
};

class serverVars : public Gtk::TreeView
{
public:
	serverVars();
	~serverVars() {}
	void init(Glib::RefPtr <Gtk::Builder> _refBuilder);
	Glib::RefPtr <Gtk::Builder> get_xml() { return refBuilder; }
	void set_editable(bool set);
	void add(const Glib::ustring& variable, const Glib::ustring& value);
	void update(const Glib::ustring& variable, const Glib::ustring& value);
	gint get_n_variables();
	Glib::ustring get_variable(gint idx);
	Glib::ustring get_value(gint idx);
	void set_value(gint idx, Glib::ustring val);
	
	sigc::signal<void, Glib::ustring> on_variable_changed;
	
protected:
	struct varColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
  	Gtk::TreeModelColumn<Glib::ustring> variable;
  	Gtk::TreeModelColumn<Glib::ustring> value;
		Gtk::TreeModelColumn<Glib::ustring> def_val;
		Gtk::TreeModelColumn<Glib::ustring> isdef_val;

  	varColumns() { add(variable); add(value); add(def_val); add(isdef_val); }
	};
	
	void on_column_value_edited(const Glib::ustring& path, const Glib::ustring& new_text);
	void on_vars_tab_close_button_pressed();
	void on_cell_data(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter);

private:
	Glib::RefPtr <Gtk::Builder> refBuilder;
	Gtk::TreeView* view;
	const varColumns columns;
	Glib::RefPtr<Gtk::ListStore> vars;
	Gtk::CellRendererText* val_col;
	Gtk::CellRendererText* var_col;
	Gtk::CellRendererText* def_col;
	Gtk::CellRendererText* isdef_col;
	
	typedef std::vector<Variable> VarVal;
	VarVal variables;
	int idx;
};

inline gint serverVars::get_n_variables()
{
	gint n = variables.size();
	if (n == 0)
		return -1;
	return n;
}

#endif // _server_vars_h_
