/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <dfaure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <xmltree.h>

#include <qdatetime.h>
#include <kdebug.h>


XMLTree::XMLTree(const QString & inputFileName)
{
  root = new QDomDocument("spreadsheet");

  root->appendChild( root->createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  doc = root->createElement( "spreadsheet" );

  doc.setAttribute("editor", "KSpread CSV Filter");
  doc.setAttribute("mime", "application/x-kspread");
  root->appendChild(doc);

  QDomElement paper = root->createElement("paper");
  paper.setAttribute("format", "A4");
  paper.setAttribute("orientation", "Portrait");
  QDomElement borders = root->createElement( "borders" );
  borders.setAttribute( "left", 20 );
  borders.setAttribute( "top", 20 );
  borders.setAttribute( "right", 20 );
  borders.setAttribute( "bottom", 20 );
  paper.appendChild( borders );
  doc.appendChild(paper);

  map = root->createElement("map");
  doc.appendChild(map);

  table = root->createElement("table");

  table.setAttribute("name", inputFileName);
  map.appendChild(table);

  row = 1;
  column = 1;
}

XMLTree::~XMLTree() 
{
  if(root)
    delete root;
}

const QString XMLTree::part()
{
  QString s;
  QTextStream t(s, IO_WriteOnly);

  QTime tmr;
  tmr.start();
  kdebug(KDEBUG_INFO, 31000, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

  root->save(t);  // Why does this take sooooo long (approx. 8s on my Athlon 500 :( )

  kdebug(KDEBUG_INFO, 31000, (const char*)QString::number((long)tmr.elapsed()));
  kdebug(KDEBUG_INFO, 31000, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

  t << '\0';

  return s;
}

bool XMLTree::cell( const QString & contents )
{
  QDomElement e = root->createElement("cell");
  //e.appendChild(getFormat(xf));
  //e.appendChild(getFont(xf));

  e.setAttribute("row", row);
  e.setAttribute("column", column++);

  QDomElement format=root->createElement("format");
  format.setAttribute("float", "3");
  format.setAttribute("alignY", "2");
  format.setAttribute("floatcolor", "2");
  format.setAttribute("faktor", "1");
  format.setAttribute("precision", "-1");
  format.setAttribute("align", "4");

  QDomElement pen=root->createElement("pen");
  pen.setAttribute("width", "1");
  pen.setAttribute("style", "0");
  pen.setAttribute("color", "#000000");

  QDomElement lborder=root->createElement("left-border");
  lborder.appendChild(pen);
  format.appendChild(lborder);

  pen=root->createElement("pen");
  pen.setAttribute("width", "1");
  pen.setAttribute("style", "0");
  pen.setAttribute("color", "#000000");

  QDomElement tborder=root->createElement("top-border");
  tborder.appendChild(pen);
  format.appendChild(tborder);

  pen=root->createElement("pen");
  pen.setAttribute("width", "1");
  pen.setAttribute("style", "0");
  pen.setAttribute("color", "#000000");

  QDomElement fdia=root->createElement("fall-diagonal");
  fdia.appendChild(pen);
  format.appendChild(fdia);

  pen=root->createElement("pen");
  pen.setAttribute("width", "1");
  pen.setAttribute("style", "0");
  pen.setAttribute("color", "#000000");

  QDomElement udia=root->createElement("up-diagonal");
  udia.appendChild(pen);
  format.appendChild(udia);

  e.appendChild(format);

  QDomElement text=root->createElement("text");
  text.appendChild(root->createTextNode(contents));
  e.appendChild(text);

  table.appendChild(e);

  return true;
}
