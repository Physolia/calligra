/*
 *  kimageshop_config.cc - part of KImageShop
 *
 *  A global configuration class for KImageShop
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <kconfig.h>
#include <kglobal.h>

#include "kimageshop_config.h"


// define static members
bool 			KImageShopConfig::doInit 	= true;
KConfig *		KImageShopConfig::kc 		= 0L;
QList<KImageShopConfig>	KImageShopConfig::instanceList;
QFont 			KImageShopConfig::m_smallFont;
QFont 			KImageShopConfig::m_tinyFont;


KImageShopConfig * KImageShopConfig::getNewConfig()
{
  if ( doInit ) {
    return ( new KImageShopConfig() );
  }
  
  else {
    if ( instanceList.count() == 0 ) {
      debug("oups, KImageShopConfig is already initialized, but instanceList is empty?!?!");
      return ( new KImageShopConfig() );
    }

    return ( new KImageShopConfig( *instanceList.first() ) );
  }
}


KImageShopConfig::KImageShopConfig() : QObject( 0L, "kimageshop config" )
{
  kc = KGlobal::config();

  // load and init global settings only once
  if ( doInit ) {
    instanceList.clear();
    loadGlobalSettings();

    doInit = false;
  }

  instanceList.append( this );

  m_pLayerDlgConfig = new LayerDlgConfig( this );
  m_pBrushDlgConfig = new BrushDlgConfig( this );
  m_pColorDlgConfig = new ColorDlgConfig( this );
  m_pGradientDlgConfig = new GradientDlgConfig( this );
  m_pGradientEditorConfig = new GradientEditorConfig( this );
  // ...


  // now load all the settings for the config objects
  loadConfig();
}


KImageShopConfig::KImageShopConfig( const KImageShopConfig& config )
{
  instanceList.append( this );

  m_pLayerDlgConfig = new LayerDlgConfig( *m_pLayerDlgConfig );
  m_pBrushDlgConfig = new BrushDlgConfig( *m_pBrushDlgConfig );
  m_pColorDlgConfig = new ColorDlgConfig( *m_pColorDlgConfig );
  m_pGradientDlgConfig = new GradientDlgConfig( *m_pGradientDlgConfig );
  m_pGradientEditorConfig = new GradientEditorConfig(*m_pGradientEditorConfig);
  // ...
}


KImageShopConfig::~KImageShopConfig()
{
  instanceList.remove( this );

  // when the last document is closed, save the global settings
  if ( instanceList.isEmpty() )
    saveGlobalSettings();

  delete m_pLayerDlgConfig;
  delete m_pBrushDlgConfig;
  delete m_pColorDlgConfig;
  delete m_pGradientDlgConfig;
  delete m_pGradientEditorConfig;
  // ...
}


// a convenience method - load all document specific configuration
void KImageShopConfig::loadConfig()
{
  loadDialogSettings();
  // ...
}


// save all document specific configuration
void KImageShopConfig::saveConfig()
{
  saveDialogSettings();
  // ...
}


void KImageShopConfig::saveAll()
{
  KImageShopConfig *config = 0L;
  for ( config = instanceList.first(); config = instanceList.next(); config ) {
    config->saveConfig();
  }

  saveGlobalSettings();
}


void KImageShopConfig::loadGlobalSettings()
{
  // read some fonts
  QFont font = KGlobal::generalFont();
  font.setPointSize( 10 );
  m_smallFont = kc->readFontEntry( "Small Font", &font );

  font = KGlobal::generalFont();
  font.setPointSize( 8 );
  m_tinyFont = kc->readFontEntry( "Tiny Font", &font );

  // ...
}


void KImageShopConfig::saveGlobalSettings()
{
  kc->setGroup( "General Settings" );

  kc->writeEntry( "Small Font", m_smallFont );
  kc->writeEntry( "Tiny Font", m_tinyFont );

  // ...
}


void KImageShopConfig::loadDialogSettings()
{
  m_pLayerDlgConfig->loadConfig( kc );
  m_pBrushDlgConfig->loadConfig( kc );
  m_pColorDlgConfig->loadConfig( kc );
  m_pGradientDlgConfig->loadConfig( kc );
  m_pGradientEditorConfig->loadConfig( kc );
  // ...
}


void KImageShopConfig::saveDialogSettings()
{
  m_pLayerDlgConfig->saveConfig( kc );
  m_pBrushDlgConfig->saveConfig( kc );
  m_pColorDlgConfig->saveConfig( kc );
  m_pGradientDlgConfig->saveConfig( kc );
  m_pGradientEditorConfig->saveConfig( kc );
  // ...
}


// The base configuration class



void BaseKFDConfig::loadConfig( KConfig *_config )
{
  m_docked = _config->readBoolEntry( "Docked", true );
  m_posX   = _config->readUnsignedNumEntry( "PositionX", 0 );
  m_posY   = _config->readUnsignedNumEntry( "PositionY", 0 );
}

void BaseKFDConfig::saveConfig( KConfig *_config )
{
  // TODO: save the right values

  _config->writeEntry( "Docked", false );
  _config->writeEntry( "PositionX", 50 );
  _config->writeEntry( "PositionY", 50 );
  
  _config->sync();
}

// the configuration classes

LayerDlgConfig::LayerDlgConfig( QObject *parent, const char *name )
{

}

LayerDlgConfig::LayerDlgConfig( const LayerDlgConfig& config )
{

}

void LayerDlgConfig::loadConfig( KConfig *kc )
{
  kc->setGroup( "LayerDialog Settings" );
  BaseKFDConfig::loadConfig( kc );

}

void LayerDlgConfig::saveConfig( KConfig *kc )
{
  kc->setGroup( "LayerDialog Settings" );
  BaseKFDConfig::saveConfig( kc );

}

//////

BrushDlgConfig::BrushDlgConfig( QObject *parent, const char *name )
{

}

BrushDlgConfig::BrushDlgConfig( const BrushDlgConfig& config )
{

}

void BrushDlgConfig::loadConfig( KConfig *kc )
{
  kc->setGroup( "BrushDialog Settings" );
  BaseKFDConfig::loadConfig( kc );

}

void BrushDlgConfig::saveConfig( KConfig *kc )
{
  kc->setGroup( "BrushDialog Settings" );
  BaseKFDConfig::saveConfig( kc );

}

//////

ColorDlgConfig::ColorDlgConfig( QObject *parent, const char *name )
{

}

ColorDlgConfig::ColorDlgConfig( const ColorDlgConfig& config )
{

}

void ColorDlgConfig::loadConfig( KConfig *kc )
{
  kc->setGroup( "ColorDialog Settings" );
  BaseKFDConfig::loadConfig( kc );

}

void ColorDlgConfig::saveConfig( KConfig *kc )
{
  kc->setGroup( "ColorDialog Settings" );
  BaseKFDConfig::saveConfig( kc );

}

//////

GradientDlgConfig::GradientDlgConfig( QObject *parent, const char *name )
{

}

GradientDlgConfig::GradientDlgConfig( const GradientDlgConfig& config )
{

}

void GradientDlgConfig::loadConfig( KConfig *kc )
{
  kc->setGroup( "GradientDialog Settings" );
  BaseKFDConfig::loadConfig( kc );

}

void GradientDlgConfig::saveConfig( KConfig *kc )
{
  kc->setGroup( "GradientDialog Settings" );
  BaseKFDConfig::saveConfig( kc );

}

//////

GradientEditorConfig::GradientEditorConfig( QObject *parent, const char *name )
{

}

GradientEditorConfig::GradientEditorConfig( const GradientEditorConfig& config)
{

}

void GradientEditorConfig::loadConfig( KConfig *kc )
{
  kc->setGroup( "GradientEditor Settings" );
  BaseKFDConfig::loadConfig( kc );

}

void GradientEditorConfig::saveConfig( KConfig *kc )
{
  kc->setGroup( "GradientEditor Settings" );
  BaseKFDConfig::saveConfig( kc );

}


#include "kimageshop_config.moc"
