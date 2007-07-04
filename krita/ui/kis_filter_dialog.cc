/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_filter_dialog.h"

#include "kis_filter.h"
#include "kis_filter_config_widget.h"
#include "kis_filter_mask.h"
#include "kis_layer.h"

#include "ui_wdgfilterdialog.h"

struct KisFilterDialog::Private {
    Private() : currentCentralWidget(0), currentFilterConfigurationWidget(0),
            currentFilter(0), layer(0), mask(0)
    {
    }
    ~Private()
    {
        delete currentCentralWidget;
        delete widgetLayout;
    }
    QWidget* currentCentralWidget;
    KisFilterConfigWidget* currentFilterConfigurationWidget;
    KisFilterSP currentFilter;
    KisLayerSP layer;
    Ui_FilterDialog uiFilterDialog;
    KisFilterMaskSP mask;
    QGridLayout *widgetLayout;
};

KisFilterDialog::KisFilterDialog(QWidget* parent, KisLayerSP layer ) :
    QDialog( parent ),
    d( new Private )
{
    setModal( false );
    d->uiFilterDialog.setupUi( this );
    d->widgetLayout = new QGridLayout( d->uiFilterDialog.centralWidgetHolder );
    d->layer = layer;
    d->mask = new KisFilterMask();
    d->layer->setPreviewMask( d->mask );
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed ()), SLOT(accept()));
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed ()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonApply, SIGNAL(pressed ()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonCancel, SIGNAL(pressed ()), SLOT(reject()));
}

KisFilterDialog::~KisFilterDialog()
{
    delete d;
}

void KisFilterDialog::setFilter(KisFilterSP f)
{
    d->currentFilter = f;
    delete d->currentCentralWidget;
    KisFilterConfigWidget* widget = d->currentFilter->createConfigurationWidget( d->uiFilterDialog.centralWidgetHolder, d->layer->paintDevice() );
    if(not widget)
    {
        d->currentFilterConfigurationWidget = 0;
        d->currentCentralWidget = new QLabel( i18n("No configuration option."), d->uiFilterDialog.centralWidgetHolder );
    } else {
        d->currentFilterConfigurationWidget = widget;
        d->currentCentralWidget = widget;
        d->currentFilterConfigurationWidget->setConfiguration( d->currentFilter->defaultConfiguration( d->layer->paintDevice() ) );
        connect(d->currentFilterConfigurationWidget, SIGNAL(sigPleaseUpdatePreview()), SLOT(updatePreview()));
    }
    d->widgetLayout->addWidget( d->currentCentralWidget, 0 , 0);
    d->uiFilterDialog.centralWidgetHolder->setMinimumSize( d->currentCentralWidget->minimumSize() );
    updatePreview();
}

void KisFilterDialog::updatePreview()
{
    if( not d->currentFilter ) return;
    if( d->currentFilterConfigurationWidget )
    {
        KisFilterConfiguration* config = d->currentFilterConfigurationWidget->configuration();
        d->mask->setFilter( config );
    } else {
        d->mask->setFilter( d->currentFilter->defaultConfiguration( d->layer->paintDevice() ) );
    }
    d->mask->setDirty();
}

void KisFilterDialog::apply()
{
    if( not d->currentFilter ) return;
    KisFilterConfiguration* config = 0;
    if( d->currentFilterConfigurationWidget )
    {
        config = d->currentFilterConfigurationWidget->configuration();
    }
    emit(sigPleaseApplyFilter(d->layer, config));
}

#include "kis_filter_dialog.moc"
