// gbzadmin
// GTKmm bzadmin
// Copyright (c) 2005 - 2012 Michael Sheppard
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

#include "server_vars_view.h"
#include "common.h"

Variable::Variable()
{
	_var = "";
	_val = "";
	_def = "";
}

Variable::Variable(Glib::ustring var, Glib::ustring val)
{
	_var = var;
	_val = val;
}

serverVars::serverVars() : Gtk::TreeView()
{
	val_col = 0;
	view = 0;
	idx = 0;
}

void serverVars::init(Glib::RefPtr <Gtk::Builder> _refBuilder)
{
	refBuilder = _refBuilder;
	
	dbItems.init();
	
	refBuilder->get_widget("server_view", view);
	
	vars = Gtk::ListStore::create(columns);
	
	view->set_model(vars);

	// Variable name column
	Gtk::TreeView::Column* pViewColumn = Gtk::manage(new Gtk::TreeView::Column("Variable", columns.variable));
  Gtk::CellRenderer* pCellRenderer = pViewColumn->get_first_cell_renderer();
  var_col = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);

	if (var_col) {
		var_col->property_foreground() = VARIABLE;
		var_col->property_background() = BKGRND;
		var_col->property_font() = FONT;
	}
	view->append_column(*pViewColumn);

	// Value column
	pViewColumn = Gtk::manage(new Gtk::TreeView::Column("Value", columns.value));
  pCellRenderer = pViewColumn->get_first_cell_renderer();
  val_col = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  
  if(val_col) {
		val_col->property_foreground() = VALUE;
		val_col->property_background() = BKGRND;
		val_col->property_font() = FONT;
		// set the editable property to false until admin rights have been established
		val_col->property_editable() = false;
	  // connect to the "edited" signal, sending the model_column too
		val_col->signal_edited().connect(sigc::mem_fun(*this, &serverVars::on_column_value_edited));
		// setup a custom cell function
		pViewColumn->set_cell_data_func(*pCellRenderer, sigc::mem_fun(*this, &serverVars::on_cell_data));
	}
	view->append_column(*pViewColumn);

	// default value column
	pViewColumn = Gtk::manage(new Gtk::TreeView::Column("Default Value", columns.def_val));
  pCellRenderer = pViewColumn->get_first_cell_renderer();
  def_col = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  
  if(def_col) {
		def_col->property_foreground() = DEFAULT;
		def_col->property_background() = BKGRND;
		def_col->property_font() = FONT;
	}
	view->append_column(*pViewColumn);
	
	// is-value-default column, this column is not visible. This column is used to
	// color code the variable values, DEFAULT if default, CHANGED if not default.
	// default server variable values are in default_vars.h. These are only 2.0 defaults.
	pViewColumn = Gtk::manage(new Gtk::TreeView::Column("isDefault Value", columns.isdef_val));
  pCellRenderer = pViewColumn->get_first_cell_renderer();
  isdef_col = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  
	view->set_headers_visible(true);
	Glib::RefPtr<Gtk::TreeSelection> sel = view->get_selection();
	sel->set_mode(Gtk::SELECTION_SINGLE);

	// connect the close button signal handler
	Gtk::Button *button;
	refBuilder->get_widget("vars_tab_close_button", button);
	button->signal_clicked().connect((sigc::mem_fun(*this, &serverVars::on_vars_tab_close_button_pressed)));
}

// callback to color code the value string depending on default value or not
void serverVars::on_cell_data(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeRow row = *iter;
	Glib::ustring str(row[columns.isdef_val]);
	Gtk::CellRendererText* cr = dynamic_cast<Gtk::CellRendererText*>(cell);
	if (str == Glib::ustring("true")) {
		cr->property_foreground() = VALUE;
	} else {
		cr->property_foreground() = CHANGED;
	}
}

void serverVars::on_vars_tab_close_button_pressed()
{
	Gtk::Notebook *nb;
	refBuilder->get_widget("notebook", nb);
	Gtk::Widget *page = nb->get_nth_page(Page_ServerVars);
	
	page->hide();
}

void serverVars::set_editable(bool set)
{
	val_col->property_editable() = set;
}

// add a single variable/value pair
void serverVars::add(const Glib::ustring& variable, const Glib::ustring& value)
{
	// add the variable/value pair to the vector
	variables.push_back(Variable(variable, value));

	// add the variable/value pair to the liststore
	Gtk::TreeRow row = *(vars->append());
	row[columns.variable] = variable;
	row[columns.value] = value;

	Glib::ustring def = dbItems.find(variable);
	row[columns.def_val] = def;

	if ((value != def) && (def != Glib::ustring("none"))) {
		row[columns.isdef_val] = Glib::ustring("false");
	} else {
		row[columns.isdef_val] = Glib::ustring("true");
	}

	idx = (idx < maxIdx) ? idx + 1 : 0; // over-run protection
}

// add or update a single variable/value pair
void serverVars::update(const Glib::ustring& variable, const Glib::ustring& value)
{
	// check to see if the variable/value pair exists
	bool updated = false;
	int len = variables.size();
	for (int k = 0; k < len; k++) {
		if (variables.at(k)._var == variable) {
			// update the value in the vector
			variables.at(k)._val = value;
			// update the value in the ListStore
			Glib::ustring path;
			path = Glib::ustring::compose("%1", k);
			Gtk::TreeRow row = *(vars->get_iter(path));
			row[columns.value] = value;

//			Glib::ustring def(defaults[k]);
			Glib::ustring def = dbItems.find(variable);
			if ((value != def) && (def != Glib::ustring("none"))) {
				row[columns.isdef_val] = Glib::ustring("false");
			} else {
				row[columns.isdef_val] = Glib::ustring("true");
			}

			updated = true;
			break;
		}
	}
	if (!updated) {
		add(variable, value);
	}
}

// this function modifies the list store
void serverVars::on_column_value_edited(const Glib::ustring& path, const Glib::ustring& new_text)
{
	Gtk::TreeRow row = *(vars->get_iter(path));
	row[columns.value] = new_text;
	Glib::ustring cmd_str("/set ");
	cmd_str += row[columns.variable] + " \"" + new_text + "\"\n";
	// emit a variable changed signal
	on_variable_changed(cmd_str);
}

Glib::ustring serverVars::get_variable(gint idx)
{
	Glib::ustring str;
	str = variables[idx]._var;
	
	return str;
}

Glib::ustring serverVars::get_value(gint idx)
{
	Glib::ustring str;
	str = variables[idx]._val;
	
	return str;
}

void serverVars::set_value(gint idx, Glib::ustring val)
{
	variables[idx]._val = val;
}

