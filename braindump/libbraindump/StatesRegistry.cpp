/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "StatesRegistry.h"

#include <QDomDocument>
#include <QFile>
#include <QSvgRenderer>

#include <kcomponentdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <QFileInfo>
#include <QDir>

State::State( const QString& _id, const QString& _name, StateCategory* _category, const QString& _fileName, int _priority) : m_id(_id), m_name(_name), m_category(_category), m_render(new QSvgRenderer(_fileName)), m_priority(_priority)
{
}

State::~State() {
  delete m_render;
}

const QString& State::name() const {
  return m_name;
}

const QString& State::id() const {
  return m_id;
}

const StateCategory* State::category() const {
  return m_category;
}

QSvgRenderer* State::renderer() const {
  return m_render;
}

int State::priority() const {
  return m_priority;
}

struct StateCategory::Private {
  QString id, name;
  QMap<QString, const State*> states;
  int priority;
};

StateCategory::StateCategory( const QString& _id, const QString& _name, int _priority) : d(new Private) {
  d->id = _id;
  d->name = _name;
  d->priority = _priority;
}

StateCategory::~StateCategory() {
  delete d;
}

const QString& StateCategory::name() const {
  return d->name;
}

const QString& StateCategory::id() const {
  return d->id;
}

QList<QString> StateCategory::stateIds() const {
  return d->states.keys();
}

const State* StateCategory::state(const QString& _id) const {
  if(d->states.contains(_id)) return d->states[_id];
  kWarning() << "No shape " << _id << " found in category " << name() << " choices: " << d->states.keys();
  return 0;
}

int StateCategory::priority() const {
  return d->priority;
}

struct StatesRegistry::Private {
  static StatesRegistry* s_instance;
  QMap<QString, StateCategory*> categories;
  void parseStatesRC(const QString& _filename );
};

StatesRegistry* StatesRegistry::Private::s_instance = 0;

void StatesRegistry::Private::parseStatesRC(const QString& _filename )
{
  QDomDocument doc;
  QFile file(_filename);
  if(not file.open(QIODevice::ReadOnly) )
  {
    kError() << "Can't open " << _filename;
    return;
  }
  QString errMsg;
  int line, column;
  if( not doc.setContent(&file, &errMsg, &line, &column) ) {
    kError() << "At (" << line << ", " << column << ") " << errMsg;
    file.close();
    return;
  }
  file.close();
  
  QDir directory = QFileInfo(_filename).absoluteDir();
  
  QDomElement docElem = doc.documentElement();
  if(docElem.nodeName() != "states") {
    kError() << "Invalid state file";
    return;
  }
  QDomNode nCat = docElem.firstChild();
  while(not nCat.isNull()) {
    QDomElement eCat = nCat.toElement(); // try to convert the node to an element.
    if(not eCat.isNull() and eCat.tagName() == "category")
    {
      QString catId = eCat.attribute("id");
      QString catName = eCat.attribute("name");
      int catPriority = eCat.attribute("priority", "1000").toInt();
      StateCategory* category = 0;
      if(catId.isEmpty()) {
        kError() << "Missing category id";
      } else {
        if( categories.contains(catId) )
        {
          category = categories[catId];
        } else if( not catName.isEmpty() ) {
          category = new StateCategory(catId, i18n(catName.toUtf8()), catPriority );
          categories[catId] = category;
        }
        if(category){
          // Parse the states
          QDomNode nState = eCat.firstChild();
          while(not nState.isNull())
          {
            QDomElement eState = nState.toElement();
            if(not eState.isNull() and eState.tagName() == "state")
            {
              QString stateId = eState.attribute("id");
              QString stateName = eState.attribute("name");
              QString stateFilename = eState.attribute("filename");
              int statePriority = eState.attribute("priority", "1000").toInt();
              if(stateId.isEmpty() or stateName.isEmpty() or stateFilename.isEmpty())
              {
                kError() << "Missing attribute: id = " << stateId << " name = " << stateName << " filename = " << stateFilename;
              } else {
                QString file = directory.absoluteFilePath(stateFilename);
                if(QFileInfo(file).exists())
                {
                  if(category->d->states.contains(stateId))
                  {
                    delete category->d->states[stateId];
                  }
                  kDebug() << "Adding state id = " << stateId << " name = " << stateName << " filename = " << stateFilename;
                  category->d->states[stateId] = new State(stateId, stateName, category, file, statePriority);
                } else {
                  kError() << "Missing file " << file;
                }
              }
            } else {
              kError() << "Invalid node in category " << catId;
            }
            nState = nState.nextSibling();
          }
        } else {
          kError() << "Couldn't make a category for " << catId;
        }
      }
    } else {
      kError() << "Invalid XML node.";
    }
    nCat = nCat.nextSibling();
  }
}

StatesRegistry::StatesRegistry() : d(new Private) {
  KGlobal::mainComponent().dirs()->addResourceType("stateshape_states", "data", "stateshape/states/");
  QStringList statesFilenames = KGlobal::mainComponent().dirs()->findAllResources("stateshape_states", "*.rc",  KStandardDirs::Recursive);

  foreach(const QString& filename, statesFilenames)
  {
    kDebug() << "Load state: " << filename;
    d->parseStatesRC(filename);
  }
}

const StatesRegistry* StatesRegistry::instance() {
  if(not Private::s_instance)
  {
    Private::s_instance = new StatesRegistry;
  }
  return Private::s_instance;
}

QList<QString> StatesRegistry::categorieIds() const {
  return d->categories.keys();
}

QList<QString> StatesRegistry::stateIds(const QString& _id) const {
  Q_ASSERT(d->categories.contains(_id));
  return d->categories[_id]->stateIds();
}

const State* StatesRegistry::state(const QString& _category, const QString& _state) const {
  if(d->categories.contains(_category)) return d->categories[_category]->state(_state);
  kWarning() << "No category " << _category << " found among " << d->categories.keys();
  return 0;
}

const State* StatesRegistry::nextState(const State* _state) const {
  if(_state) {
    QList<const State*> states = _state->category()->d->states.values();
    int idx = states.indexOf(_state);
    idx += 1;
    if( idx >= states.count() ) idx = 0;
    return states[idx];
  }
  return 0;
}
