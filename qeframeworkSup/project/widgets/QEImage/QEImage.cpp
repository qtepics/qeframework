/*  QEimage.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2020 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class is a CA aware image widget based on the Qt frame widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details.

  This class displays images from byte array (originating from a EPICS waveform record)
  It determines the width and height from other EPICS variables.
  The user can interact with the image.
  The image is managed by the VideoWidget class.
  User interaction and drawing markups over the image (such as selecting an area) is managed by the imageMarkup class.
 */

#include <QEImage.h>
#include <QDebug>
#include <QIcon>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QECommon.h>
#include <profilePlot.h>
#include <QEByteArray.h>
#include <QEInteger.h>
#include <QEFloating.h>
#include <QEString.h>
#include <QENTNDArrayData.h>
#include <imageContextMenu.h>
#include <windowCustomisation.h>
#include <screenSelectDialog.h>
#include <colourConversion.h>

#define DEBUG qDebug() << "QEImage"  << __LINE__ << __FUNCTION__ << "  "


/*
    Constructor with no initialisation
*/
QEImage::QEImage( QWidget *parent ) : QFrame( parent ), QEWidget( this ) {
    setup();
}

/*
    Constructor with known variable
*/
QEImage::QEImage( const QString &variableNameIn, QWidget *parent ) : QFrame( parent ), QEWidget( this )  {
    setup();
    setVariableName( variableNameIn, 0 );
    activate();
}

/*
    Setup common to all constructors
*/
void QEImage::setup()
{
    // Create MPEG data source - may be a stubb depending
    mpegSource = new MpegSource( this );

    // Set up data
    // This control uses the following data sources:
    //  - image
    //  - width
    //  - height
    setNumVariables( QEIMAGE_NUM_VARIABLES );

    // Set up default properties
    setVariableAsToolTip( false );

    setAllowDrop( false );

    resizeOption = RESIZE_OPTION_ZOOM;
    zoom = 100;
    XStretch = 1.0;
    YStretch = 1.0;
    infoUpdateZoom( zoom, XStretch, YStretch );
    imageSizeSet = false;

    initialHozScrollPos = 0;
    initialVertScrollPos = 0;
    initScrollPosSet = false;

    paused = false;
    infoUpdatePaused( paused );
    pauseExternalAction = NULL;

    vSlice1Thickness = 1;
    vSlice2Thickness = 1;
    vSlice3Thickness = 1;
    vSlice4Thickness = 1;
    vSlice5Thickness = 1;

    hSlice1Thickness = 1;
    hSlice2Thickness = 1;
    hSlice3Thickness = 1;
    hSlice4Thickness = 1;
    hSlice5Thickness = 1;
    profileThickness = 1;

    vSlice1X = 0;
    vSlice2X = 0;
    vSlice3X = 0;
    vSlice4X = 0;
    vSlice5X = 0;

    hSlice1Y = 0;
    hSlice2Y = 0;
    hSlice3Y = 0;
    hSlice4Y = 0;
    hSlice5Y = 0;

    haveVSlice1X = false;
    haveVSlice2X = false;
    haveVSlice3X = false;
    haveVSlice4X = false;
    haveVSlice5X = false;

    haveHSlice1Y = false;
    haveHSlice2Y = false;
    haveHSlice3Y = false;
    haveHSlice4Y = false;
    haveHSlice5Y = false;

    haveProfileLine = false;

    haveSelectedArea1 = false;
    haveSelectedArea2 = false;
    haveSelectedArea3 = false;
    haveSelectedArea4 = false;

    enableProfilePresentation = true;
    enableHozSlicePresentation = true;
    enableVertSlicePresentation = true;

    // Set the initial state
    lastSeverity = QCaAlarmInfo::getInvalidSeverity();
    isConnected = false;

    appHostsControls = false;
    hostingAppAvailable = false;

    displayMarkups = false;

    fullScreen = false;
    fullScreenMainWindow = NULL;

    ellipseVariableUsage = CENTRE_AND_SIZE;

    imageUse = IMAGE_USE_DISPLAY;

    // With so many variables involved, don't bother alterning the presentation
    // of the widget when any one variable goes into alarm
    setDisplayAlarmStateOption( DISPLAY_ALARM_STATE_NEVER );

    // Prepare to interact with whatever application is hosting this widget.
    // For example, the QEGui application can host docks and toolbars for QE widgets
    if( isProfileDefined() )
    {
        // Setup a signal to request component hosting.
        QObject* launcher = getGuiLaunchConsumer();
        if( launcher )
        {
            QObject::connect( this, SIGNAL( componentHostRequest( const QEActionRequests& ) ),
                              launcher, SLOT( requestAction( const QEActionRequests& ) ) );
            hostingAppAvailable = true;
        }
    }

    // Connect MPEG data source.
    QObject::connect (mpegSource, SIGNAL (setDataImage( const QByteArray&,
                                                        unsigned long, unsigned long,
                                                        unsigned long, unsigned long,
                                                        imageDataFormats::formatOptions, unsigned int )),
                      this,       SLOT   (setDataImage( const QByteArray&,
                                                        unsigned long, unsigned long,
                                                        unsigned long, unsigned long,
                                                        imageDataFormats::formatOptions, unsigned int )));

    // Create the video destination
    videoWidget = new VideoWidget( this );
    setVertSlice1MarkupColor( QColor(127, 255, 127));
    setVertSlice2MarkupColor( QColor(114, 230, 114));  // 90% of slice 1
    setVertSlice3MarkupColor( QColor(101, 204, 101));  // 80% of slice 1
    setVertSlice4MarkupColor( QColor( 89, 179,  89));  // 70% of slice 1
    setVertSlice5MarkupColor( QColor( 76, 153,  76));  // 60% of slice 1
    setHozSlice1MarkupColor(  QColor(255, 100, 100));
    setHozSlice2MarkupColor(  QColor(230,  90,  90));  // 90% of slice 1
    setHozSlice3MarkupColor(  QColor(204,  80,  80));  // 80% of slice 1
    setHozSlice4MarkupColor(  QColor(179,  70,  70));  // 70% of slice 1
    setHozSlice5MarkupColor(  QColor(153,  60,  60));  // 60% of slice 1
    setProfileMarkupColor(   QColor(255, 255, 100));
    setAreaMarkupColor(      QColor(100, 100, 255));
    setBeamMarkupColor(      QColor(255,   0,   0));
    setTargetMarkupColor(    QColor(  0, 255,   0));
    setTimeMarkupColor(      QColor(255, 255, 255));
    setEllipseMarkupColor(   QColor(255, 127, 255));

    QObject::connect( videoWidget, SIGNAL( userSelection( imageMarkup::markupIds, bool, bool, QPoint, QPoint, unsigned int ) ),
                      this,        SLOT  ( userSelection( imageMarkup::markupIds, bool, bool, QPoint, QPoint, unsigned int )) );
    QObject::connect( videoWidget, SIGNAL( zoomInOut( int ) ),
                      this,        SLOT  ( zoomInOut( int ) ) );
    QObject::connect( videoWidget, SIGNAL( currentPixelInfo( QPoint ) ),
                      this,        SLOT  ( currentPixelInfo( QPoint ) ) );
    QObject::connect( videoWidget, SIGNAL( pan( QPoint ) ),
                      this,        SLOT  ( pan( QPoint ) ) );
    QObject::connect( videoWidget, SIGNAL( redraw() ),
                      this,        SLOT  ( redraw() ) );


    // Create zoom sub menu
    zMenu = new zoomMenu();
    zMenu->enableAreaSelected( haveSelectedArea1 );
    QObject::connect( zMenu, SIGNAL( triggered ( QAction* ) ), this,  SLOT  ( zoomMenuTriggered( QAction* )) );

    // Create flip/rotate sub menu
    frMenu = new flipRotateMenu();
    frMenu->setChecked( iProcessor.getRotation(), iProcessor.getFlipHoz(), iProcessor.getFlipVert() );
    QObject::connect( frMenu, SIGNAL( triggered ( QAction* ) ), this,  SLOT  ( flipRotateMenuTriggered( QAction* )) );

    // Create and setup the select menu
    sMenu = new selectMenu();
    QObject::connect( sMenu, SIGNAL( triggered ( QAction* ) ), this,  SLOT  ( selectMenuTriggered( QAction* )) );

    // Create and setup the markup display menu
    mdMenu = new markupDisplayMenu();
    QObject::connect( mdMenu, SIGNAL( triggered ( QAction* ) ), this,  SLOT  ( markupDisplayMenuTriggered( QAction* )) );

    // Add the video destination to the widget
    scrollArea = new QScrollArea;
    scrollArea->setFrameStyle( QFrame::NoFrame );
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget( videoWidget );

    // Image display properties controls
    imageDisplayProps = new imageDisplayProperties;

    iProcessor.setImageDisplayProperties( imageDisplayProps );

    QObject::connect( imageDisplayProps, SIGNAL( imageDisplayPropertiesChange() ),
                      this,    SLOT  ( imageDisplayPropertiesChanged()) );
    QObject::connect( imageDisplayProps, SIGNAL( brightnessContrastAutoImage() ),
                      this,    SLOT  ( brightnessContrastAutoImageRequest() ) );
    QObject::connect(imageDisplayProps, SIGNAL(destroyed(QObject*)), this, SLOT(imageDisplayPropsDestroyed(QObject*)));

    // Create image recorder
    recorder = new recording( this );
    QObject::connect(recorder, SIGNAL(destroyed(QObject*)), this, SLOT(recorderDestroyed(QObject*)));
    QObject::connect(recorder, SIGNAL(playingBack(bool)), this, SLOT(playingBack(bool)));
    QObject::connect( recorder,  SIGNAL( byteArrayChanged( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                      this, SLOT( setImage( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );

    // Create vertical, horizontal, and general profile plots
    vSliceLabel = new QLabel( "Vertical Profile" );
    vSliceLabel->setVisible( false );
    vSliceDisplay = new profilePlot( profilePlot::PROFILEPLOT_BT );
    QObject::connect(vSliceDisplay, SIGNAL(destroyed(QObject*)), this, SLOT(vSliceDisplayDestroyed(QObject*)));
    vSliceDisplay->setVisible( false );

    hSliceLabel = new QLabel( "Horizontal Profile" );
    hSliceLabel->setVisible( false );
    hSliceDisplay = new profilePlot( profilePlot::PROFILEPLOT_LR );
    QObject::connect(hSliceDisplay, SIGNAL(destroyed(QObject*)), this, SLOT(hSliceDisplayDestroyed(QObject*)));
    hSliceDisplay->setVisible( false );

    profileLabel = new QLabel( "Arbitrary Line Profile" );
    profileLabel->setVisible( false );
    profileDisplay = new profilePlot( profilePlot::PROFILEPLOT_LR );
    QObject::connect(profileDisplay, SIGNAL(destroyed(QObject*)), this, SLOT(profileDisplayDestroyed(QObject*)));
    profileDisplay->setVisible( false );

    graphicsLayout = new QGridLayout();
    graphicsLayout->addWidget( scrollArea,      0, 0 );
    graphicsLayout->addLayout( getInfoWidget(), 1, 0 );
    graphicsLayout->addWidget( vSliceLabel,    1, 1 );
    graphicsLayout->addWidget( hSliceLabel,    2, 0 );
    graphicsLayout->addWidget( profileLabel,   4, 0 );

    // Create button group
    int buttonWidth = 28;
    int buttonMenuWidth = 48;

    buttonGroup = new QFrame;
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setMargin( 0 );
    buttonGroup->setLayout( buttonLayout );

    pauseButton = new QPushButton( buttonGroup );
    pauseButton->setMinimumWidth( buttonWidth );
    pauseButtonIcon = new QIcon( ":/qe/image/pause.png" );
    playButtonIcon = new QIcon( ":/qe/image/play.png" );
    pauseButton->setIcon( *pauseButtonIcon );
    pauseButton->setToolTip("Pause image display");
    QObject::connect(pauseButton, SIGNAL(clicked()), this, SLOT(pauseClicked()));

    saveButton = new QPushButton(buttonGroup);
    saveButton->setMinimumWidth( buttonWidth );
    QIcon saveButtonIcon( ":/qe/image/save.png" );
    saveButton->setIcon( saveButtonIcon );
    saveButton->setToolTip("Save displayed image");
    QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(saveClicked()));

    targetButton = new QPushButton(buttonGroup);
    targetButton->setMinimumWidth( buttonWidth );
    QIcon targetButtonIcon( ":/qe/image/target.png" );
    targetButton->setIcon( targetButtonIcon );
    targetButton->setToolTip("Move target position into beam");
    QObject::connect(targetButton, SIGNAL(clicked()), this, SLOT(targetClicked()));

    selectModeButton = new QPushButton(buttonGroup);
    selectModeButton->setMinimumWidth( buttonMenuWidth );
    QIcon selectModeButtonIcon( ":/qe/image/select.png" );
    selectModeButton->setIcon( selectModeButtonIcon );
    selectModeButton->setToolTip("Choose selection and pan modes");
    selectModeButton->setMenu( sMenu );

    markupDisplayButton = new QPushButton(buttonGroup);
    markupDisplayButton->setMinimumWidth( buttonMenuWidth );
    QIcon markupDisplayButtonIcon( ":/qe/image/markupDisplay.png" );
    markupDisplayButton->setIcon( markupDisplayButtonIcon );
    markupDisplayButton->setToolTip("Hide or reveal markups");
    markupDisplayButton->setMenu( mdMenu );

    zoomButton = new QPushButton(buttonGroup);
    zoomButton->setMinimumWidth( buttonMenuWidth );
    QIcon zoomButtonIcon( ":/qe/image/zoom.png" );
    zoomButton->setIcon( zoomButtonIcon );
    zoomButton->setToolTip("Zoom options");
    zoomButton->setMenu( zMenu );

    flipRotateButton = new QPushButton(buttonGroup);
    flipRotateButton->setMinimumWidth( buttonMenuWidth );
    QIcon flipRotateButtonIcon( ":/qe/image/flipRotate.png" );
    flipRotateButton->setIcon( flipRotateButtonIcon );
    flipRotateButton->setToolTip("Flip and rotate options");
    flipRotateButton->setMenu( frMenu );


    buttonLayout->addWidget( pauseButton,         0);
    buttonLayout->addWidget( saveButton,          1);
    buttonLayout->addWidget( targetButton,        2);
    buttonLayout->addWidget( selectModeButton,    3);
    buttonLayout->addWidget( markupDisplayButton, 4);
    buttonLayout->addWidget( zoomButton,          5);
    buttonLayout->addWidget( flipRotateButton,    6);
    buttonLayout->addStretch();


    // Create main layout containing image, label, and button layouts
    mainLayout = new QGridLayout;
    mainLayout->setMargin( 0 );
    graphicsLayout->addWidget( vSliceLabel,    1, 1 );
    graphicsLayout->addWidget( hSliceLabel,    2, 0 );
    graphicsLayout->addWidget( profileLabel,   4, 0 );

    mainLayout->addWidget( buttonGroup, 2, 0 );

    presentControls();

    graphicsLayout->setColumnStretch( 0, 1 );  // display image to take all spare room
    graphicsLayout->setRowStretch( 0, 1 );  // display image to take all spare room

    mainLayout->addLayout( graphicsLayout, 3, 0, 1, 0 );

    // Set graphics to take all spare room
    mainLayout->setColumnStretch( 0, 1 );
    mainLayout->setRowStretch( 3, 1 );

    setLayout( mainLayout );

    // Set up context sensitive menu (right click menu)
    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, SIGNAL( customContextMenuRequested( const QPoint& )), this, SLOT( showImageContextMenu( const QPoint& )));
    fullContextMenu = true;

    // Create options dialog
    // This is done after all items manipulated by the options dialog have been built - such as the brightness/contrast controls
    // Also pareneted by this so will scaled automatically.
    optionsDialog = new QEImageOptionsDialog( this );
    QObject::connect( optionsDialog, SIGNAL( optionChange(  imageContextMenu::imageContextMenuOptions, bool )),
                      this,          SLOT  ( optionAction( imageContextMenu::imageContextMenuOptions, bool )) );
    optionsDialog->initialise();

    // Initially set the video widget to the size of the scroll bar
    // This will be resized when the image size is known
    videoWidget->resize( scrollArea->width(), scrollArea->height() );

    // Simulate pan mode being selected
    panModeClicked();
    sMenu->setChecked( QEImage::SO_PANNING );

    // Connect to the image process to be able to receive images as they are built from image data
    QObject::connect( &iProcessor, SIGNAL( imageBuilt( QImage, QString ) ), this, SLOT( displayBuiltImage( QImage, QString ) ) );

    // !! move this functionality into QEWidget???
    // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
    // for each variable name property manager, set up an index to identify it when it signals and
    // set up a connection to recieve variable name property changes.
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    for( int i = 0; i < QEIMAGE_NUM_VARIABLES; i++ )
    {
        variableNamePropertyManagers[i].setVariableIndex( i );
        QObject::connect( &variableNamePropertyManagers[i], SIGNAL( newVariableNameProperty( QString, QString, unsigned int ) ), this, SLOT( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
    }
}

QEImage::~QEImage()
{
    // Release components hosted by the application.
    // Note, the application may already have deleted them in which case we will
    // have recieved a destroyed signal and set the reference to the component to NULL.
    // An example of this scenario is if a QEGui main window is closed while a GUI is displayed.
    // Components not hosted by the application will be part of the widget hierarchy under this
    // widget and will not need explicit deletion.
    if( appHostsControls && hostingAppAvailable )
    {
        if( imageDisplayProps )
        {
            QObject::disconnect( imageDisplayProps, 0, this, 0);
            delete imageDisplayProps;
        }

        if( vSliceDisplay )
        {
            QObject::disconnect( vSliceDisplay, 0, this, 0);
            delete vSliceDisplay;
        }

        if( hSliceDisplay )
        {
            QObject::disconnect( hSliceDisplay, 0, this, 0);
            delete hSliceDisplay;
        }

        if( profileDisplay )
        {
            QObject::disconnect( profileDisplay, 0, this, 0);
            delete profileDisplay;
        }

        if( recorder )
        {
            QObject::disconnect( recorder, 0, this, 0);
            delete recorder;
        }
    }
    delete videoWidget;
}

// If an object handed over to the application (which we have a reference to) has been deleted, then clear the reference
void QEImage::imageDisplayPropsDestroyed( QObject* )
{
    imageDisplayProps = NULL;
    iProcessor.setImageDisplayProperties( NULL );
}
void QEImage::vSliceDisplayDestroyed( QObject* ){ vSliceDisplay = NULL; }
void QEImage::hSliceDisplayDestroyed( QObject* ){ hSliceDisplay = NULL; }
void QEImage::profileDisplayDestroyed( QObject* ){ profileDisplay = NULL; }
void QEImage::recorderDestroyed( QObject* ){ recorder = NULL; }

// Put the controls where they should go.
// (within this widget, or hosted by the application containing this widget)
void QEImage::presentControls()
{
    // Build the title prefix (if requried) for any external controls hosted by the application.
    QString name = getName();
    if (!name.isEmpty())
    {
        name.append(" - ");
    }

    // If components are being hosted by the application, hide any associated labels within the widget and
    // hand over the components to the application.
    // (Note, if components are not being hosted, they should always exist, but if something
    //  has gone wrong perhaps the appliction has deleted them, so don't assume they are present)
    if( appHostsControls && hostingAppAvailable )
    {
        QList<componentHostListItem> components;

        if( imageDisplayProps )
        {
            mainLayout->removeWidget( imageDisplayProps );
            components.append( componentHostListItem( imageDisplayProps, QEActionRequests::OptionFloatingDockWindow, true, name + "Image Display Properties" ) );
        }

        if( recorder )
        {
            mainLayout->removeWidget( recorder );
            components.append( componentHostListItem( recorder, QEActionRequests::OptionFloatingDockWindow, true, name + "Recorder" ) );
        }

        vSliceLabel->setVisible( false );
        hSliceLabel->setVisible( false );
        profileLabel->setVisible( false );

        if( vSliceDisplay && enableVertSlicePresentation )
        {
            graphicsLayout->removeWidget( vSliceDisplay );
            components.append( componentHostListItem( vSliceDisplay,  QEActionRequests::OptionLeftDockWindow, true, name + "Vertical Slice Profile" ) );
        }

        if( hSliceDisplay && enableHozSlicePresentation )
        {
            graphicsLayout->removeWidget( hSliceDisplay );
            components.append( componentHostListItem( hSliceDisplay,  QEActionRequests::OptionTopDockWindow, true, name + "Horizontal Slice Profile" ) );
        }

        if( profileDisplay && enableProfilePresentation )
        {
            graphicsLayout->removeWidget( profileDisplay );
            components.append( componentHostListItem( profileDisplay, QEActionRequests::OptionTopDockWindow, true, name + "Arbitrary Profile" ) );
        }

        buttonGroup->hide();

        emitComponentHostRequest( QEActionRequests( components ) );

    }

    // If components are not being hosted by the application, present them within the widget.
    // (Note, if components are not being hosted, they should always exist, but if something
    //  has gone wrong perhaps the appliction has deleted them, so don't assume they are present)
    else
    {
        if( imageDisplayProps )
        {
            mainLayout->addWidget( imageDisplayProps, 0, 0 );
        }

        if( recorder )
        {
            mainLayout->addWidget( recorder, 1, 0 );
        }

        if( vSliceDisplay && enableVertSlicePresentation )
        {
            graphicsLayout->addWidget( vSliceDisplay,  0, 1 );
            vSliceLabel->setVisible( vSliceDisplay->isVisible() );
        }
        if( hSliceDisplay && enableHozSlicePresentation )
        {
            graphicsLayout->addWidget( hSliceDisplay,  3, 0 );
            hSliceLabel->setVisible( hSliceDisplay->isVisible() );
        }
        if( profileDisplay && enableProfilePresentation )
        {
            graphicsLayout->addWidget( profileDisplay, 5, 0 );
            profileLabel->setVisible( profileDisplay->isVisible() );
        }
    }
}

/*
    Implementation of QEWidget's virtual funtion to create the specific types of QCaObject required.
*/
qcaobject::QCaObject* QEImage::createQcaItem( unsigned int variableIndex ) {

    switch( (variableIndexes)variableIndex )
    {
        // Create the image item as a QEByteArray
        case IMAGE_VARIABLE:
            {
                // Create the image item
                QEByteArray* qca = new QEByteArray( getSubstitutedVariableName( variableIndex ), this, variableIndex );

                int elementCount = iProcessor.getElementCount();
                if( elementCount )
                {
                    qca->setRequestedElementCount( elementCount );
                }
                return qca;
            }

        // Create the image format as a QEString
        case FORMAT_VARIABLE:
        case DATA_TYPE_VARIABLE:
            return new QEString( getSubstitutedVariableName( variableIndex ), this, &stringFormatting, variableIndex );

        // Create the image dimensions, target and beam, regions and profile, clipping items and other variables as a QEInteger
        case BIT_DEPTH_VARIABLE:

        case NUM_DIMENSIONS_VARIABLE:
        case DIMENSION_0_VARIABLE:
        case DIMENSION_1_VARIABLE:
        case DIMENSION_2_VARIABLE:

        case WIDTH_VARIABLE:
        case HEIGHT_VARIABLE:

        case ROI1_X_VARIABLE:
        case ROI1_Y_VARIABLE:
        case ROI1_W_VARIABLE:
        case ROI1_H_VARIABLE:

        case ROI2_X_VARIABLE:
        case ROI2_Y_VARIABLE:
        case ROI2_W_VARIABLE:
        case ROI2_H_VARIABLE:

        case ROI3_X_VARIABLE:
        case ROI3_Y_VARIABLE:
        case ROI3_W_VARIABLE:
        case ROI3_H_VARIABLE:

        case ROI4_X_VARIABLE:
        case ROI4_Y_VARIABLE:
        case ROI4_W_VARIABLE:
        case ROI4_H_VARIABLE:

        case TARGET_X_VARIABLE:
        case TARGET_Y_VARIABLE:

        case BEAM_X_VARIABLE:
        case BEAM_Y_VARIABLE:
        case TARGET_TRIGGER_VARIABLE:

        case CLIPPING_ONOFF_VARIABLE:
        case CLIPPING_LOW_VARIABLE:
        case CLIPPING_HIGH_VARIABLE:

        case PROFILE_H1_VARIABLE:
        case PROFILE_H1_THICKNESS_VARIABLE:
        case PROFILE_H2_VARIABLE:
        case PROFILE_H2_THICKNESS_VARIABLE:
        case PROFILE_H3_VARIABLE:
        case PROFILE_H3_THICKNESS_VARIABLE:
        case PROFILE_H4_VARIABLE:
        case PROFILE_H4_THICKNESS_VARIABLE:
        case PROFILE_H5_VARIABLE:
        case PROFILE_H5_THICKNESS_VARIABLE:

        case PROFILE_V1_VARIABLE:
        case PROFILE_V1_THICKNESS_VARIABLE:
        case PROFILE_V2_VARIABLE:
        case PROFILE_V2_THICKNESS_VARIABLE:
        case PROFILE_V3_VARIABLE:
        case PROFILE_V3_THICKNESS_VARIABLE:
        case PROFILE_V4_VARIABLE:
        case PROFILE_V4_THICKNESS_VARIABLE:
        case PROFILE_V5_VARIABLE:
        case PROFILE_V5_THICKNESS_VARIABLE:

        case LINE_PROFILE_X1_VARIABLE:
        case LINE_PROFILE_Y1_VARIABLE:
        case LINE_PROFILE_X2_VARIABLE:
        case LINE_PROFILE_Y2_VARIABLE:
        case LINE_PROFILE_THICKNESS_VARIABLE:

        case ELLIPSE_X_VARIABLE:
        case ELLIPSE_Y_VARIABLE:
        case ELLIPSE_W_VARIABLE:
        case ELLIPSE_H_VARIABLE:

            return new QEInteger( getSubstitutedVariableName( variableIndex ), this, &integerFormatting, variableIndex );

        case PROFILE_H_ARRAY:
        case PROFILE_V_ARRAY:
        case PROFILE_LINE_ARRAY:
        case ELLIPSE_R_VARIABLE:

            return new QEFloating( getSubstitutedVariableName( variableIndex ), this, &floatingFormatting, variableIndex );

        default:
            return NULL;
    }
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QEImage::establishConnection( unsigned int variableIndex ) {

    // IMAGE_VARIABLE width and height are available check has been moved to processing.

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    switch( (variableIndexes)variableIndex )
    {
        // Connect the image waveform record to the display image
        case IMAGE_VARIABLE:
            if( qca )
            {
                QObject::connect( qca,  SIGNAL( byteArrayChanged( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setImage( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );

                // Note: we connect to receive the 'raw' variant data for PVA image data
                //
                QObject::connect (qca, SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                                  this, SLOT  (setPvaImage (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));

                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        case FORMAT_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setFormat( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect the bit depth variable
        case BIT_DEPTH_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setBitDepth( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect the data type variable
        case DATA_TYPE_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setDataType( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect the image dimension variables
        case WIDTH_VARIABLE:
        case HEIGHT_VARIABLE:
        case NUM_DIMENSIONS_VARIABLE:
        case DIMENSION_0_VARIABLE:
        case DIMENSION_1_VARIABLE:
        case DIMENSION_2_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setDimension( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect the clipping variables
        case CLIPPING_ONOFF_VARIABLE:
        case CLIPPING_LOW_VARIABLE:
        case CLIPPING_HIGH_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setClipping( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Conntect to the ROI variables
        case ROI1_X_VARIABLE:
        case ROI1_Y_VARIABLE:
        case ROI1_W_VARIABLE:
        case ROI1_H_VARIABLE:

        case ROI2_X_VARIABLE:
        case ROI2_Y_VARIABLE:
        case ROI2_W_VARIABLE:
        case ROI2_H_VARIABLE:

        case ROI3_X_VARIABLE:
        case ROI3_Y_VARIABLE:
        case ROI3_W_VARIABLE:
        case ROI3_H_VARIABLE:

        case ROI4_X_VARIABLE:
        case ROI4_Y_VARIABLE:
        case ROI4_W_VARIABLE:
        case ROI4_H_VARIABLE:

            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setROI( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect to line profile variables
        case PROFILE_H1_VARIABLE:
        case PROFILE_H1_THICKNESS_VARIABLE:
        case PROFILE_H2_VARIABLE:
        case PROFILE_H2_THICKNESS_VARIABLE:
        case PROFILE_H3_VARIABLE:
        case PROFILE_H3_THICKNESS_VARIABLE:
        case PROFILE_H4_VARIABLE:
        case PROFILE_H4_THICKNESS_VARIABLE:
        case PROFILE_H5_VARIABLE:
        case PROFILE_H5_THICKNESS_VARIABLE:
        case PROFILE_V1_VARIABLE:
        case PROFILE_V1_THICKNESS_VARIABLE:
        case PROFILE_V2_VARIABLE:
        case PROFILE_V2_THICKNESS_VARIABLE:
        case PROFILE_V3_VARIABLE:
        case PROFILE_V3_THICKNESS_VARIABLE:
        case PROFILE_V4_VARIABLE:
        case PROFILE_V4_THICKNESS_VARIABLE:
        case PROFILE_V5_VARIABLE:
        case PROFILE_V5_THICKNESS_VARIABLE:
        case LINE_PROFILE_X1_VARIABLE:
        case LINE_PROFILE_Y1_VARIABLE:
        case LINE_PROFILE_X2_VARIABLE:
        case LINE_PROFILE_Y2_VARIABLE:
        case LINE_PROFILE_THICKNESS_VARIABLE:

            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setProfile( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Connect to targeting variables
        case TARGET_X_VARIABLE:
        case TARGET_Y_VARIABLE:

        case BEAM_X_VARIABLE:
        case BEAM_Y_VARIABLE:

            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setTargeting( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Don't connect to target trigger variable.
        // We are not interested in updates from it - we just write to it when the user clicks on the target trigger button
        case TARGET_TRIGGER_VARIABLE:
            break;

        // QCa creation occured, but no connection for display is required here.
        case PROFILE_H_ARRAY:
        case PROFILE_V_ARRAY:
        case PROFILE_LINE_ARRAY:

            break;

        // Connect to ellipse variables
        case ELLIPSE_X_VARIABLE:
        case ELLIPSE_Y_VARIABLE:
        case ELLIPSE_W_VARIABLE:
        case ELLIPSE_H_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setEllipse( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        case ELLIPSE_R_VARIABLE:
            if(  qca )
            {
                QObject::connect( qca,  SIGNAL( floatingChanged( const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                                  this, SLOT( setEllipseFloat(   const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
                QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                                  this, SLOT  ( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
                QObject::connect( this, SIGNAL( requestResend() ),
                                  qca, SLOT( resendLastData() ) );
            }
            break;

        // Not a variable index. Included to avoid compilation warnings
        case QEIMAGE_NUM_VARIABLES:
            break;

     }
}

/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QEImage::connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex )
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    qcaobject::QCaObject* qca = getQcaItem( variableIndex );
    switch( (variableIndexes)variableIndex )
    {
        // Connect the image waveform record to the display image
        case IMAGE_VARIABLE:
            this->isFirstImageUpdate = true;
            if( qca && qca->isPvaChannel() ){
                // PVA channel suppied as QENTImageData varient.
                //
                qca->setSignalsToSend( qcaobject::QCaObject::SIG_VARIANT );
            }
            break;

        default:
            // no action
            break;
    }


// Don't perform standard connection action (grey out widget and all its dialogs, and place disconnected in tooltip)
// If
//    // Display the connected state
//    updateToolTipConnection( isConnected );
//    processConnectionInfo( isConnected );

// Instead just log the disconnected variables.
    if( !isConnected )
    {
        QString messageText;
        messageText.append( "Disconnected variable: " ).append( connectionInfo.variable() );
        sendMessage( messageText, "QEImage" );
    }

}

/*
    Update the image format from a variable.
    This tends to take precedence over the format property simply as variable data arrives after all properties are set.
    If the 'format' property is set later, then it be used.

    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setFormat( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex )
{
    // Sanity check - Only deal with format variable
    if( variableIndex != FORMAT_VARIABLE)
    {
        return;
    }

    // Set the format based on the area detector format text
    if( !iProcessor.setFormat( text ) )
    {
        return;
    }

    // Update the image.
    // This is required if image data arrived before the format.
    // The image data will be present, but will not have been used to update the image if the
    // width and height and format were not available at the time of the image update.
    displayImage();

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

/*
    Update the image dimensions (width and height in various arrangements)
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setDimension( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex )
{
    bool dimensionChange = false;

    // C/C++ not so smart assiging signed long value to an
    // unsigned long value - ensure sensible.
    unsigned long uValue = MAX( 0, value );

    // Update image size variable
    switch( variableIndex )
    {
        case WIDTH_VARIABLE:          dimensionChange = iProcessor.setWidth( uValue );         break;
        case HEIGHT_VARIABLE:         dimensionChange = iProcessor.setHeight( uValue );        break;
        case NUM_DIMENSIONS_VARIABLE: dimensionChange = iProcessor.setNumDimensions( uValue ); break;
        case DIMENSION_0_VARIABLE:    dimensionChange = iProcessor.setDimension0( uValue );    break;
        case DIMENSION_1_VARIABLE:    dimensionChange = iProcessor.setDimension1( uValue );    break;
        case DIMENSION_2_VARIABLE:    dimensionChange = iProcessor.setDimension2( uValue );    break;
    }

    // Update the image buffer according to the new size
    // This will do nothing unless both width and height are available
    setImageSize();

    // Update the image.
    // This is required if image data for an enlarged image arrived before the width and height.
    // The image data will be present, but will not have been used to update the image if the
    // width and height were not suitable at the time of the image update
    displayImage();

    // If the image size or data array dimensions has changed and we have good dimensions, update the image
    // variable connection to reflect the elements we now need.
    if( dimensionChange && iProcessor.validateDimensions() )
    {
        // Re-establish the image connection. This will set request the appropriate array size.
        establishConnection( IMAGE_VARIABLE );
    }

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

/*
    Update the image dimensions (bit depth)
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setBitDepth( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // Sanity check - Only deal with bit depth
    if( variableIndex != BIT_DEPTH_VARIABLE)
    {
        return;
    }

    // Update the depth
    setBitDepth( value );

    // Update the image.
    // This is required if image data for an enlarged image arrived before the width and height.
    // The image data will be present, but will not have been used to update the image if the
    // width and height were not suitable at the time of the image update
    displayImage();

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

/*
    Update the image dimensions (bit depth derived from data type)
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setDataType( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex )
{
    // Sanity check - Only deal with data type
    if( variableIndex != DATA_TYPE_VARIABLE)
    {
        return;
    }

    // Determine the bit depth from the data type.
    long value = 1;
    if(      text == "Int8" )    value = 7;
    else if( text == "UInt8" )   value = 8;
    else if( text == "Int16" )   value = 15;
    else if( text == "UInt16" )  value = 16;
    else if( text == "Int32" )   value = 24; // Todo:??? Should be 31. change to 31 when all pixel information is held in unsigned int or long. (mostly in brightness / contrast code where int is used to woek well with sliders)
    else if( text == "UInt32" )  value = 24; // Todo:??? Should be 32. change to 32 when all pixel information is held in unsigned int or long. (mostly in brightness / contrast code where int is used to woek well with sliders)
    else if( text == "Float32" ) value = 16; // Bit depth cannot be derived. Assume 16 bit (actually, setting bit depth from the data type is always an assumption!)
    else if( text == "Float64" ) value = 16; // Bit depth cannot be derived. Assume 16 bit (actually, setting bit depth from the data type is always an assumption!)
    else return;

    // Update the depth
    setBitDepth( value );

    // Update the image.
    // This is required if image data for an enlarged image arrived before the width and height.
    // The image data will be present, but will not have been used to update the image if the
    // width and height were not suitable at the time of the image update
    displayImage();

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

/*
    Update the clipping info
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setClipping( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // Update image size variable
    switch( variableIndex )
    {
        case CLIPPING_ONOFF_VARIABLE: iProcessor.setClippingOn(   (value>0)?true:false ); break;
        case CLIPPING_LOW_VARIABLE:   iProcessor.setClippingLow(  (unsigned int)value );  break;
        case CLIPPING_HIGH_VARIABLE:  iProcessor.setClippingHigh( (unsigned int)value );  break;
    }

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

/*
    Update the ROI displays if any
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setROI( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // If invalid, mark the appropriate ROI info as not present
    if( alarmInfo.isInvalid() )
    {
        switch( variableIndex )
        {
            case ROI1_X_VARIABLE: roiInfo[0].clearX(); break;
            case ROI1_Y_VARIABLE: roiInfo[0].clearY(); break;
            case ROI1_W_VARIABLE: roiInfo[0].clearW(); break;
            case ROI1_H_VARIABLE: roiInfo[0].clearH(); break;
            case ROI2_X_VARIABLE: roiInfo[1].clearX(); break;
            case ROI2_Y_VARIABLE: roiInfo[1].clearY(); break;
            case ROI2_W_VARIABLE: roiInfo[1].clearW(); break;
            case ROI2_H_VARIABLE: roiInfo[1].clearH(); break;
            case ROI3_X_VARIABLE: roiInfo[2].clearX(); break;
            case ROI3_Y_VARIABLE: roiInfo[2].clearY(); break;
            case ROI3_W_VARIABLE: roiInfo[2].clearW(); break;
            case ROI3_H_VARIABLE: roiInfo[2].clearH(); break;
            case ROI4_X_VARIABLE: roiInfo[3].clearX(); break;
            case ROI4_Y_VARIABLE: roiInfo[3].clearY(); break;
            case ROI4_W_VARIABLE: roiInfo[3].clearW(); break;
            case ROI4_H_VARIABLE: roiInfo[3].clearH(); break;
            }
    }

    // Good data. Save the ROI data (and note it is present) then if the
    // markup is visible, update it
    else
    {
        // Save the tageting data
        switch( variableIndex )
        {
            case ROI1_X_VARIABLE:  roiInfo[0].setX( value ); break;
            case ROI1_Y_VARIABLE:  roiInfo[0].setY( value ); break;
            case ROI1_W_VARIABLE:  roiInfo[0].setW( value ); break;
            case ROI1_H_VARIABLE:  roiInfo[0].setH( value ); break;
            case ROI2_X_VARIABLE:  roiInfo[1].setX( value ); break;
            case ROI2_Y_VARIABLE:  roiInfo[1].setY( value ); break;
            case ROI2_W_VARIABLE:  roiInfo[1].setW( value ); break;
            case ROI2_H_VARIABLE:  roiInfo[1].setH( value ); break;
            case ROI3_X_VARIABLE:  roiInfo[2].setX( value ); break;
            case ROI3_Y_VARIABLE:  roiInfo[2].setY( value ); break;
            case ROI3_W_VARIABLE:  roiInfo[2].setW( value ); break;
            case ROI3_H_VARIABLE:  roiInfo[2].setH( value ); break;
            case ROI4_X_VARIABLE:  roiInfo[3].setX( value ); break;
            case ROI4_Y_VARIABLE:  roiInfo[3].setY( value ); break;
            case ROI4_W_VARIABLE:  roiInfo[3].setW( value ); break;
            case ROI4_H_VARIABLE:  roiInfo[3].setH( value ); break;
        }

        // If there is an image, present the ROI data
        // (if there is no image, the ROI data will be used when one arrives)
        if( videoWidget->hasCurrentImage() )
        {
            useROIData( variableIndex );
        }
    }
}

// Apply the ROI data.
// This can be done once all ROI data is available and an image is available
// (the image is needed to determine scaling)
void QEImage::useROIData( const unsigned int& variableIndex )
{
#define USE_ROI_DATA( IS_ENABLED, IS_DISPLAY, N )                                               \
    if( sMenu->isEnabled( imageContextMenu::IS_ENABLED ) &&                                     \
        mdMenu->isDisplayed( imageContextMenu::IS_DISPLAY ) &&                                  \
        roiInfo[N].getStatus() )                                                                \
    {                                                                                           \
        QRect rotateFlipArea = iProcessor.rotateFlipToImageRectangle( roiInfo[N].getArea() );   \
        videoWidget->markupRegionValueChange( N, rotateFlipArea, displayMarkups );              \
    }                                                                                           \
    break;


    switch( variableIndex )
    {
        case ROI1_X_VARIABLE:
        case ROI1_Y_VARIABLE:
        case ROI1_W_VARIABLE:
        case ROI1_H_VARIABLE:
            USE_ROI_DATA( ICM_SELECT_AREA1, ICM_DISPLAY_AREA1, 0 )

        case ROI2_X_VARIABLE:
        case ROI2_Y_VARIABLE:
        case ROI2_W_VARIABLE:
        case ROI2_H_VARIABLE:
            USE_ROI_DATA( ICM_SELECT_AREA2,  ICM_DISPLAY_AREA2, 1 )

        case ROI3_X_VARIABLE:
        case ROI3_Y_VARIABLE:
        case ROI3_W_VARIABLE:
        case ROI3_H_VARIABLE:
            USE_ROI_DATA( ICM_SELECT_AREA3,  ICM_DISPLAY_AREA3, 2 )

        case ROI4_X_VARIABLE:
        case ROI4_Y_VARIABLE:
        case ROI4_W_VARIABLE:
        case ROI4_H_VARIABLE:
            USE_ROI_DATA( ICM_SELECT_AREA4,  ICM_DISPLAY_AREA4, 3 )
    }
}

/*
    Update the Profile displays if any
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setProfile( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // If invalid, mark the appropriate profile info as not present
    if( alarmInfo.isInvalid() )
    {
        switch( variableIndex )
        {
            case PROFILE_H1_VARIABLE:             hSlice1Y = 0;              break;
            case PROFILE_H2_VARIABLE:             hSlice2Y = 0;              break;
            case PROFILE_H3_VARIABLE:             hSlice3Y = 0;              break;
            case PROFILE_H4_VARIABLE:             hSlice4Y = 0;              break;
            case PROFILE_H5_VARIABLE:             hSlice5Y = 0;              break;
            case PROFILE_V1_VARIABLE:             vSlice1X = 0;              break;
            case PROFILE_V2_VARIABLE:             vSlice2X = 0;              break;
            case PROFILE_V3_VARIABLE:             vSlice3X = 0;              break;
            case PROFILE_V4_VARIABLE:             vSlice4X = 0;              break;
            case PROFILE_V5_VARIABLE:             vSlice5X = 0;              break;
            case PROFILE_H1_THICKNESS_VARIABLE:   hSlice1Thickness = 1;      break;
            case PROFILE_H2_THICKNESS_VARIABLE:   hSlice2Thickness = 1;      break;
            case PROFILE_H3_THICKNESS_VARIABLE:   hSlice3Thickness = 1;      break;
            case PROFILE_H4_THICKNESS_VARIABLE:   hSlice4Thickness = 1;      break;
            case PROFILE_H5_THICKNESS_VARIABLE:   hSlice5Thickness = 1;      break;
            case PROFILE_V1_THICKNESS_VARIABLE:   vSlice1Thickness = 1;      break;
            case PROFILE_V2_THICKNESS_VARIABLE:   vSlice2Thickness = 1;      break;
            case PROFILE_V3_THICKNESS_VARIABLE:   vSlice3Thickness = 1;      break;
            case PROFILE_V4_THICKNESS_VARIABLE:   vSlice4Thickness = 1;      break;
            case PROFILE_V5_THICKNESS_VARIABLE:   vSlice5Thickness = 1;      break;
            case LINE_PROFILE_X1_VARIABLE:        lineProfileInfo.clearX1(); break;
            case LINE_PROFILE_Y1_VARIABLE:        lineProfileInfo.clearY1(); break;
            case LINE_PROFILE_X2_VARIABLE:        lineProfileInfo.clearX2(); break;
            case LINE_PROFILE_Y2_VARIABLE:        lineProfileInfo.clearY2(); break;
            case LINE_PROFILE_THICKNESS_VARIABLE: profileThickness = 1;      break;
        }
    }

    // Good data. Save the profile data (and note it is present) then if the
    // markup is visible, update it
    else
    {
        // Save the tageting data
        switch( variableIndex )
        {
            case PROFILE_H1_VARIABLE:             hSlice1Y = value;                break;
            case PROFILE_H2_VARIABLE:             hSlice2Y = value;                break;
            case PROFILE_H3_VARIABLE:             hSlice3Y = value;                break;
            case PROFILE_H4_VARIABLE:             hSlice4Y = value;                break;
            case PROFILE_H5_VARIABLE:             hSlice5Y = value;                break;
            case PROFILE_V1_VARIABLE:             vSlice1X = value;                break;
            case PROFILE_V2_VARIABLE:             vSlice2X = value;                break;
            case PROFILE_V3_VARIABLE:             vSlice3X = value;                break;
            case PROFILE_V4_VARIABLE:             vSlice4X = value;                break;
            case PROFILE_V5_VARIABLE:             vSlice5X = value;                break;
            case PROFILE_H1_THICKNESS_VARIABLE:   hSlice1Thickness = value;        break;
            case PROFILE_H2_THICKNESS_VARIABLE:   hSlice2Thickness = value;        break;
            case PROFILE_H3_THICKNESS_VARIABLE:   hSlice3Thickness = value;        break;
            case PROFILE_H4_THICKNESS_VARIABLE:   hSlice4Thickness = value;        break;
            case PROFILE_H5_THICKNESS_VARIABLE:   hSlice5Thickness = value;        break;
            case PROFILE_V1_THICKNESS_VARIABLE:   vSlice1Thickness = value;        break;
            case PROFILE_V2_THICKNESS_VARIABLE:   vSlice2Thickness = value;        break;
            case PROFILE_V3_THICKNESS_VARIABLE:   vSlice3Thickness = value;        break;
            case PROFILE_V4_THICKNESS_VARIABLE:   vSlice4Thickness = value;        break;
            case PROFILE_V5_THICKNESS_VARIABLE:   vSlice5Thickness = value;        break;
            case LINE_PROFILE_X1_VARIABLE:        lineProfileInfo.setX1( value );  break;
            case LINE_PROFILE_Y1_VARIABLE:        lineProfileInfo.setY1( value );  break;
            case LINE_PROFILE_X2_VARIABLE:        lineProfileInfo.setX2( value );  break;
            case LINE_PROFILE_Y2_VARIABLE:        lineProfileInfo.setY2( value );  break;
            case LINE_PROFILE_THICKNESS_VARIABLE: profileThickness = 1;            break;
        }

        // If there is an image, present the profile data
        // (if there is no image, the profile data will be used when one arrives)
        if( videoWidget->hasCurrentImage() )
        {
            useProfileData( variableIndex );
        }
    }
}

// Apply the profile data.
// This can be done once all profile data is available and an image is available
// (the image is needed to determine scaling)
void QEImage::useProfileData( const unsigned int& variableIndex )
{
    switch( variableIndex )
    {
        case PROFILE_H1_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_HSLICE1 ) )
            {
                videoWidget->markupH1ProfileChange( hSlice1Y, displayMarkups );
            }
            break;

        case PROFILE_H2_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_HSLICE2 ) )
            {
                videoWidget->markupH2ProfileChange( hSlice2Y, displayMarkups );
            }
            break;

        case PROFILE_H3_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_HSLICE3 ) )
            {
                videoWidget->markupH3ProfileChange( hSlice3Y, displayMarkups );
            }
            break;

        case PROFILE_H4_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_HSLICE4 ) )
            {
                videoWidget->markupH4ProfileChange( hSlice4Y, displayMarkups );
            }
            break;

        case PROFILE_H5_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_HSLICE5 ) )
            {
                videoWidget->markupH5ProfileChange( hSlice5Y, displayMarkups );
            }
            break;

        case PROFILE_V1_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_VSLICE1 ) )
            {
                videoWidget->markupV1ProfileChange(  vSlice1X, displayMarkups );
            }
            break;

        case PROFILE_V2_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_VSLICE2 ) )
            {
                videoWidget->markupV2ProfileChange(  vSlice2X, displayMarkups );
            }
            break;

        case PROFILE_V3_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_VSLICE3 ) )
            {
                videoWidget->markupV3ProfileChange(  vSlice3X, displayMarkups );
            }
            break;

        case PROFILE_V4_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_VSLICE4 ) )
            {
                videoWidget->markupV4ProfileChange(  vSlice4X, displayMarkups );
            }
            break;

        case PROFILE_V5_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_VSLICE5 ) )
            {
                videoWidget->markupV5ProfileChange(  vSlice5X, displayMarkups );
            }
            break;

        case LINE_PROFILE_X1_VARIABLE:
        case LINE_PROFILE_Y1_VARIABLE:
        case LINE_PROFILE_X2_VARIABLE:
        case LINE_PROFILE_Y2_VARIABLE:
            if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_PROFILE ) && lineProfileInfo.getStatus() )
            {
                videoWidget->markupLineProfileChange( iProcessor.rotateFlipToImagePoint( lineProfileInfo.getPoint1() ),
                                                      iProcessor.rotateFlipToImagePoint( lineProfileInfo.getPoint2() ),
                                                      displayMarkups );
            }
            break;
    }
}

/*
    Update the Ellipse displays if any
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setEllipse( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // If invalid, mark the appropriate profile info as not present
    if( alarmInfo.isInvalid() )
    {
        switch( variableIndex )
        {
            case ELLIPSE_X_VARIABLE: ellipseInfo.clearX(); break;
            case ELLIPSE_Y_VARIABLE: ellipseInfo.clearY(); break;
            case ELLIPSE_W_VARIABLE: ellipseInfo.clearW(); break;
            case ELLIPSE_H_VARIABLE: ellipseInfo.clearH(); break;
        }
    }

    // Good data. Save the ellipse data (and note it is present) then if the
    // markup is visible, update it
    else
    {
        // Save the ellipse data
        switch( variableIndex )
        {
            case ELLIPSE_X_VARIABLE: ellipseInfo.setX( value ); break;
            case ELLIPSE_Y_VARIABLE: ellipseInfo.setY( value ); break;
            case ELLIPSE_W_VARIABLE: ellipseInfo.setW( value ); break;
            case ELLIPSE_H_VARIABLE: ellipseInfo.setH( value ); break;
        }

        // If there is an image, present the ellipse data
        // (if there is no image, the profile data will be used when one arrives)
        if( videoWidget->hasCurrentImage() )
        {
            useEllipseData();
        }
    }
}

/*
    Update the Ellipse displays if any
    This is the slot used to recieve rotation angle - clockwise in degrees.
 */
void QEImage::setEllipseFloat( const double& value, QCaAlarmInfo& alarmInfo,
                               QCaDateTime&, const unsigned int& variableIndex)
{
    // If invalid, mark the appropriate profile info as not present
    if( alarmInfo.isInvalid() )
    {
        switch( variableIndex )
        {
            case ELLIPSE_R_VARIABLE: ellipseInfo.clearR(); break;
        }
    }

    // Good data. Save the ellipse data (and note it is present) then if the
    // markup is visible, update it
    else
    {
        // Save the ellipse data
        switch( variableIndex )
        {
            case ELLIPSE_R_VARIABLE: ellipseInfo.setR( value ); break;
        }

        // If there is an image, present the ellipse data
        // (if there is no image, the profile data will be used when one arrives)
        if( videoWidget->hasCurrentImage() )
        {
            useEllipseData();
        }
    }
}

// Apply the ellipse data.
// This can be done once all ellipse data is available and an image is available
// (the image is needed to determine scaling)
void QEImage::useEllipseData()
{
    if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_ELLIPSE ) && ellipseInfo.getStatus() )
    {
        // Get the ellipse area from the two points defining the area
        QRect area = ellipseInfo.getArea();
        switch( ellipseVariableUsage )
        {
            // The area defines a bounding rectangle
            case BOUNDING_RECTANGLE:
                // Nothing to change
                break;

            // The area defines centre and size
            case CENTRE_AND_SIZE:
                // Correct to be around centre
                area.moveCenter( area.topLeft() );
                break;
        }

        // Scale, flip, and rotate the area then display the markup and the markup rotation to match
        QRect rotateFlipArea = iProcessor.rotateFlipToImageRectangle( area );
        double rotation = ellipseInfo.getRotation();
        if( iProcessor.getFlipHoz() != iProcessor.getFlipVert() ){
            rotation = -rotation;
        }
        videoWidget->markupEllipseValueChange( rotateFlipArea.topLeft(), rotateFlipArea.bottomRight(),
                                               rotation, displayMarkups );
    }
}

/*
    Update the target and beam position markers if any.
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEImage::setTargeting( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex)
{
    // If invalid, mark the appropriate profile info as not present
    if( alarmInfo.isInvalid() )
    {
        switch( variableIndex )
        {
            case TARGET_X_VARIABLE: targetInfo.clearX();  break;
            case TARGET_Y_VARIABLE: targetInfo.clearY();  break;
            case BEAM_X_VARIABLE:   beamInfo.clearX();    break;
            case BEAM_Y_VARIABLE:   beamInfo.clearX();    break;
        }
    }

    // Good data. Save the target and beam data (and note it is present) then if the
    // markup is visible, update it
    else
    {
        // Save the tageting data
        switch( variableIndex )
        {
            case TARGET_X_VARIABLE:  targetInfo.setX( value ); break;
            case TARGET_Y_VARIABLE:  targetInfo.setY( value ); break;
            case BEAM_X_VARIABLE:    beamInfo.setX( value );   break;
            case BEAM_Y_VARIABLE:    beamInfo.setY( value );   break;
        }

        // If there is an image, present the targeting data
        // (if there is no image, the targeting data will be used when one arrives)
        if( videoWidget->hasCurrentImage() )
        {
            useTargetingData();
        }
    }
}

// Apply the targeting data.
// This can be done once all targeting data is available and an image is available
// (the image is needed to determine scaling)
void QEImage::useTargetingData()
{
    if( sMenu->isEnabled( imageContextMenu::ICM_SELECT_TARGET ) && targetInfo.getStatus() && beamInfo.getStatus() )
    {
        videoWidget->markupTargetValueChange( iProcessor.rotateFlipToImagePoint( targetInfo.getPoint() ), displayMarkups );
        videoWidget->markupBeamValueChange( iProcessor.rotateFlipToImagePoint( beamInfo.getPoint() ), displayMarkups );
    }
}

// Display all markup data
// Used When the first image update occurs to display any
// markups for which data has arrived, but could not be presented
// until an image was available to determine scaling
void QEImage::useAllMarkupData()
{
    useROIData( ROI1_X_VARIABLE );
    useROIData( ROI1_Y_VARIABLE );
    useROIData( ROI1_W_VARIABLE );
    useROIData( ROI1_H_VARIABLE );
    useROIData( ROI2_X_VARIABLE );
    useROIData( ROI2_Y_VARIABLE );
    useROIData( ROI2_W_VARIABLE );
    useROIData( ROI2_H_VARIABLE );
    useROIData( ROI3_X_VARIABLE );
    useROIData( ROI3_Y_VARIABLE );
    useROIData( ROI3_W_VARIABLE );
    useROIData( ROI3_H_VARIABLE );
    useROIData( ROI4_X_VARIABLE );
    useROIData( ROI4_Y_VARIABLE );
    useROIData( ROI4_W_VARIABLE );
    useROIData( ROI4_H_VARIABLE );

    useProfileData( PROFILE_H1_VARIABLE);
    useProfileData( PROFILE_H2_VARIABLE);
    useProfileData( PROFILE_H3_VARIABLE);
    useProfileData( PROFILE_H4_VARIABLE);
    useProfileData( PROFILE_H5_VARIABLE);
    useProfileData( PROFILE_V1_VARIABLE );
    useProfileData( PROFILE_V2_VARIABLE );
    useProfileData( PROFILE_V3_VARIABLE );
    useProfileData( PROFILE_V4_VARIABLE );
    useProfileData( PROFILE_V5_VARIABLE );
    useProfileData( LINE_PROFILE_X1_VARIABLE ); //!!! all 4 of these requried???
    useProfileData( LINE_PROFILE_Y1_VARIABLE );
    useProfileData( LINE_PROFILE_X2_VARIABLE );
    useProfileData( LINE_PROFILE_Y2_VARIABLE );

    useTargetingData(); //!!! change this to use each of the targeting

    useEllipseData();
}

//====================================================
// Slot from recorder control to indicate playback has started or stopped.
// When playing back, live sources should be stopped.
void QEImage::playingBack( bool playing )
{
    if( playing )
    {
        deleteQcaItem( IMAGE_VARIABLE, true );
        mpegSource->stopStream();
    }
    else
    {
        establishConnection( IMAGE_VARIABLE );
        mpegSource->startStream();
    }
}

//====================================================

// Update image from non CA souce (no associated CA timestamp or alarm info available)
void QEImage::setDataImage( const QByteArray& imageIn,
                            unsigned long dataSize,
                            unsigned long elements,
                            unsigned long width,
                            unsigned long height,
                            imageDataFormats::formatOptions format,
                            unsigned int depth )
{
    //!!! Should the format, bit depth, width and height be clobered like this? (especially where we are altering properties, like bitDepth)
    //!!! Perhaps CA delivered and MPEG delivered images should maintain their own attributes?

    // set the format
    setFormatOption( format );

    //!!! should also set format as delivered with image from mpeg source???

    // Set the image bit depth
    iProcessor.setBitDepth( depth );

    iProcessor.setElementsPerPixel( elements );

    // Set the image dimensions to match the image size
    iProcessor.setImageBuffWidth( width );
    iProcessor.setImageBuffHeight( height );

    // Update the image buffer according to the new size.
    setImageSize();

    // Generate QCa like alarm and time info
    QCaAlarmInfo alarmInfo;
    QCaDateTime dateTime = QCaDateTime( QDateTime::currentDateTime() );

    // Call the standard CA set image
    setImage( imageIn, dataSize, alarmInfo, dateTime, IMAGE_VARIABLE );
}

/* -----------------------------------------------------------------------------
    Update the image
    This is the slot used to recieve data updates via PV Access.
 */
void QEImage::setPvaImage( const QVariant& value,
                           QCaAlarmInfo& alarmInfo,
                           QCaDateTime& timeStamp,
                           const unsigned int& variableIndex )
{
    if (variableIndex != IMAGE_VARIABLE) {
       DEBUG << "unexpected variableIndex" << variableIndex;
       return;
    }

    QENTNDArrayData imageData;

    if (!imageData.assignFromVariant (value)) {
       if (this->isFirstImageUpdate) {
          DEBUG << "PV" << this->getSubstitutedVariableName (variableIndex)
                << "does not provides NTNDArray data";
       }
       this->isFirstImageUpdate = false;
       return;
    }

    // Decompress if needs be.
    //
    bool status = imageData.decompressData ();

    // set the format
    setFormatOption( imageData.getFormat() );

    // Set the image bit depth
    iProcessor.setBitDepth( imageData.getBitDepth() );

    iProcessor.setElementsPerPixel( imageData.getBytesPerPixel() );

    // Set the image dimensions to match the image size
    iProcessor.setImageBuffWidth( imageData.getWidth() );
    iProcessor.setImageBuffHeight( imageData.getHeight() );

    // Update the image buffer according to the new size.
    setImageSize();

    // Call the standard CA set image
    setImage( imageData.getData(), imageData.getBytesPerPixel(),
              alarmInfo, timeStamp, variableIndex );
}


/* -----------------------------------------------------------------------------
    Update the image
    This is the slot used to recieve data updates from a QCaObject based class,
    or from a non CA based data source including an MPG decoder and an image file source.
    Note the following comments from the Qt help:
        Note: Drawing into a QImage with QImage::Format_Indexed8 is not supported.
        Note: Do not render into ARGB32 images using QPainter. Using QImage::Format_ARGB32_Premultiplied is significantly faster.
 */
void QEImage::setImage( const QByteArray& imageIn,
                        unsigned long dataSize,
                        QCaAlarmInfo& alarmInfo,
                        QCaDateTime& time,
                        const unsigned int& )
{
    // Do nothing regarding the image until the width and height are available
    if( iProcessor.getImageBuffWidth() == 0 || iProcessor.getImageBuffHeight() == 0 )
    {
        return;
    }

    // If the display is paused, do nothing
    if (paused)
    {
        return;
    }

    // If recording, save image
    if( recorder && recorder->isRecording() )
    {
        recorder->recordImage( imageIn, dataSize, alarmInfo, time );
    }

    // Signal a database value change to any Link widgets
    emit dbValueChanged( "image" );

    // Save the image data for analysis and redisplay
    iProcessor.setImage( imageIn, dataSize );

    // Note the time of this image
    imageTime = time;

    // Note if the widget already had an image
    // (Used below to determine if markups data should now be applied)
    bool hasImage = videoWidget->hasCurrentImage();

    // Present the new image
    displayImage();

    // If this is the first image update, use any markup data that may have already arrived
    // (markup data can't be used until there is an image to determine the current scaling from)
    // Set of as a timer only to ensure it occurs after the initial paint already queued by displayImage() above.
    if( !hasImage )
    {
        QTimer::singleShot( 0, this, SLOT(useAllMarkupData() ) );
    }

    // Indicate another image has arrived
    freshImage( time );

    // Display invalid if invalid
    if( alarmInfo.isInvalid() )
    {
        //setImageInvalid()
        // !!! not done
    }
}

// Display a new image.
void QEImage::displayImage()
{
    // Set up the displayed image size if not done already.
    // This needs to get done once (here) initially, and is done whenever something
    // changes. For example, the user changes the zoom level.
    if( !imageSizeSet )
    {
        setImageSize();
    }

    // Now an image can be displayed, set the initial scroll bar positions if not set before
    if( initScrollPosSet == false )
    {
        scrollArea->verticalScrollBar()->setValue( initialVertScrollPos );
        scrollArea->horizontalScrollBar()->setValue( initialHozScrollPos );
        initScrollPosSet = true;
    }

    // Process the image data. Hopefully a presentable QImage will be result.
    iProcessor.buildImage();

    // Displaying the image will continue in the slot QEImage::newProcessedImage() below
}

// Continue displaying a new image.
// This slot continues the work of the function QEImage::displayImage() above.
//
void QEImage::displayBuiltImage( QImage image, QString messageText )
{
    // If there was an error processing the image, report it.
    if( !messageText.isEmpty() )
    {
        sendMessage( messageText, "QEImage" );
    }

    // If no image could be created, do nothing.
    // Even without an error message above, it may be reasonable that no image
    // can be created - for example, if image dimensions are not yet available.
    if( image.isNull() )
    {
        return;
    }

    // Display the new image
    videoWidget->setNewImage( image, imageTime );

    // Update markups if required
    updateMarkupData();

    // Display the image statistics
    imageDisplayProps->showStatistics();
}

// Return the size of the widget where the image will be presented
// It will be presented in the QEImage's main window used for full screen view,
// or in QEImage's scroll area
QSize QEImage::getVideoDestinationSize()
{
    // If full screen, return the size of the main window used for this
    // (sanity check, only do this if the full screen widget is present - it always should be in full screen)
    if( fullScreen && fullScreenMainWindow )
    {
        return fullScreenMainWindow->size();
    }
    // Not in full screen, the destination is the scroll area widget
    else
    {
        return scrollArea->size();
    }
}

// Set the video widget size so it will match the processed image.
void QEImage::setImageSize()
{
    int sx, sy;

    // Do nothing if there are no image dimensions yet
    if( !iProcessor.getImageBuffWidth() || !iProcessor.getImageBuffHeight() )
        return;

    // Size the image
    switch( resizeOption )
    {
        // Zoom the image
        case RESIZE_OPTION_ZOOM:
            sx = (double)iProcessor.rotatedImageBuffWidth()  * zoom / 100.0 * XStretch;
            sy = (double)iProcessor.rotatedImageBuffHeight() * zoom / 100.0 * YStretch;
            videoWidget->resize( sx, sy );
            break;

        // Resize the image to fit exactly within the QCaItem
        case RESIZE_OPTION_FIT:
            QSize destSize = getVideoDestinationSize();
            double vScale = (double)(destSize.height()) / (double)(iProcessor.rotatedImageBuffHeight());
            double hScale = (double)(destSize.width())  / (double)(iProcessor.rotatedImageBuffWidth());
            double scale = MIN( hScale, vScale );

            sx = (double)iProcessor.rotatedImageBuffWidth()  * scale * XStretch;
            sy = (double)iProcessor.rotatedImageBuffHeight() * scale * YStretch;
            videoWidget->resize( sx, sy );
            zoom = scale * 100;

            // Update the info area
            infoUpdateZoom( zoom, XStretch, YStretch );

            break;
    }

    // Flag the image size has been set
    imageSizeSet = true;
}

//=================================================================================================

// Allow a signal to supply a filename of an image that will be used instead of a live image
void QEImage::setImageFile( QString name )
{
    // Generate an image given the filename
    QImage image( name );

    // Generate an array of image data so the mechanisms that normally work
    // on the raw image waveform data have data to work on
    QImage stdImage = image.convertToFormat( QImage::Format_RGB32 );
#if QT_VERSION >= 0x040700
    const uchar* iDataPtr = stdImage.constBits();
#else
    const uchar* iDataPtr = stdImage.bits();
#endif
    int iDataSize = stdImage.byteCount();

    QByteArray baData;
    baData.resize( iDataSize );
    char* baDataPtr = baData.data();

    //!!! memcpy will be more efficient.
    for( int i = 0; i < iDataSize; i++ )
    {
        baDataPtr[i] = iDataPtr[i];
    }

    // Generate information normally associated with an image waveform
    QCaAlarmInfo alarmInfo;

    QFileInfo fi( name );
    QCaDateTime time = fi.lastModified();

    // Setup the widget in the same way receiving valid image, width and height data would
    // !! make this common to the actual data update functions
    scrollArea->setEnabled( true );
    iProcessor.setImageBuffWidth( stdImage.width() );
    iProcessor.setImageBuffHeight( stdImage.height() );
    iProcessor.setFormat( imageDataFormats::RGB1 );
    iProcessor.setBitDepth( 8 );


    setImageSize();

    // Use the image data just like it came from a waveform variable
    setImage( baData, 4, alarmInfo, time, 0 );
}

//=================================================================================================

// Update data related to markups if required.
// This is called after displaying the image.
void QEImage::updateMarkupData()
{
    if( haveVSlice1X )
    {
        generateVSlice( vSlice1X, vSlice1Thickness );
    }
    if( haveHSlice1Y )
    {
        generateHSlice( hSlice1Y, hSlice1Thickness );
    }
    if( haveProfileLine )
    {
        generateProfile( profileLineStart, profileLineEnd, profileThickness );
    }
    if( haveSelectedArea1 )
    {
        displaySelectedAreaInfo( 1, selectedArea1Point1, selectedArea1Point2 );
    }
    if( haveSelectedArea2 )
    {
        displaySelectedAreaInfo( 2, selectedArea2Point1, selectedArea2Point2 );
    }
    if( haveSelectedArea3 )
    {
        displaySelectedAreaInfo( 3, selectedArea3Point1, selectedArea3Point2 );
    }
    if( haveSelectedArea4 )
    {
        displaySelectedAreaInfo( 4, selectedArea4Point1, selectedArea4Point2 );
    }
}

//=================================================================================================

// Zoom to the area selected on the image
void QEImage::zoomToArea()
{
    // Determine the x and y zoom factors for the selected area
    // (the user is most likely to have selected an area with an
    // aspect ratio that does not match the current viewport)
    // Note, these zoom factors are the multiple the current zoom
    // must be changed by, not the actual zoom required
    // (Ensure at least one pixel size to avoide divide by zero)
    int sizeX = videoWidget->scaleImageOrdinate( selectedArea1Point2.x() ) - videoWidget->scaleImageOrdinate( selectedArea1Point1.x() );
    int sizeY = videoWidget->scaleImageOrdinate( selectedArea1Point2.y() ) - videoWidget->scaleImageOrdinate( selectedArea1Point1.y() );

    if( sizeX <= 0 )
    {
        sizeX = 1;
    }
    if( sizeY <= 0 )
    {
        sizeY = 1;
    }

    double zoomFactorX = (double)(scrollArea->viewport()->width()) / (double)sizeX;
    double zoomFactorY = (double)(scrollArea->viewport()->height()) / (double)sizeY;

    // Determine which of the zoom factors will display all the selected area
    double zoomFactor = std::min( zoomFactorX, zoomFactorY );

    //Determine the new zoom
    double newZoom = zoomFactor * (double)(videoWidget->width()) / (double)(iProcessor.getImageBuffWidth());

    // Ensure the zoom factor will not generate an image that is too large
    double maxDim = 5000;
    if( ((double)(iProcessor.getImageBuffWidth()) * newZoom ) > maxDim )
    {
        newZoom = (double)maxDim / (double)videoWidget->width();
    }
    if( ((double)(videoWidget->height()) * newZoom ) > maxDim )
    {
        newZoom = (double)maxDim / (double)videoWidget->height();
    }

    // Note the pixel position of the top left of the selected area in the original image
    // This will be the position that should be at the top left in the scroll area.
    QPoint newOrigin = selectedArea1Point1;

    // Resize the display widget
    int newSizeX = int( (double)(iProcessor.getImageBuffWidth()) * newZoom );
    int newSizeY = int( (double)(iProcessor.getImageBuffHeight()) * newZoom );
    videoWidget->resize( newSizeX, newSizeY );

    // Reposition the display widget
    newOrigin.setX( int( -newOrigin.x()*newZoom ) );
    newOrigin.setY( int( -newOrigin.y()*newZoom ) );
    pan( newOrigin );

    // Set current zoom percentage
    zoom = int( newZoom*100.0 );

    // Update the info area
    infoUpdateZoom( zoom, XStretch, YStretch );
}

// ROI area 1 changed
void QEImage::roi1Changed()
{
    // Write the ROI variables.
    QEInteger *qca;
    QPoint p1 = iProcessor.rotateFlipToDataPoint( selectedArea1Point1 );
    QPoint p2 = iProcessor.rotateFlipToDataPoint( selectedArea1Point2 );
    QRect r( p1, p2 );
    r = r.normalized();

    qca = (QEInteger*)getQcaItem( ROI1_X_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().x() );

    qca = (QEInteger*)getQcaItem( ROI1_Y_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().y() );

    qca = (QEInteger*)getQcaItem( ROI1_W_VARIABLE );
    if( qca ) qca->writeInteger( r.width() );

    qca = (QEInteger*)getQcaItem( ROI1_H_VARIABLE );
    if( qca ) qca->writeInteger( r.height() );

    return;
}

// ROI area 2 changed
void QEImage::roi2Changed()
{
    // Write the ROI variables.
    QEInteger *qca;
    QPoint p1 = iProcessor.rotateFlipToDataPoint( selectedArea2Point1 );
    QPoint p2 = iProcessor.rotateFlipToDataPoint( selectedArea2Point2 );
    QRect r( p1, p2 );
    r = r.normalized();

    qca = (QEInteger*)getQcaItem( ROI2_X_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().x() );

    qca = (QEInteger*)getQcaItem( ROI2_Y_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().y() );

    qca = (QEInteger*)getQcaItem( ROI2_W_VARIABLE );
    if( qca ) qca->writeInteger( r.width() );

    qca = (QEInteger*)getQcaItem( ROI2_H_VARIABLE );
    if( qca ) qca->writeInteger( r.height() );

    return;
}

// ROI area 3 changed
void QEImage::roi3Changed()
{
    // Write the ROI variables.
    QEInteger *qca;
    QPoint p1 = iProcessor.rotateFlipToDataPoint( selectedArea3Point1 );
    QPoint p2 = iProcessor.rotateFlipToDataPoint( selectedArea3Point2 );
    QRect r( p1, p2 );
    r = r.normalized();

    qca = (QEInteger*)getQcaItem( ROI3_X_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().x() );

    qca = (QEInteger*)getQcaItem( ROI3_Y_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().y() );

    qca = (QEInteger*)getQcaItem( ROI3_W_VARIABLE );
    if( qca ) qca->writeInteger( r.width() );

    qca = (QEInteger*)getQcaItem( ROI3_H_VARIABLE );
    if( qca ) qca->writeInteger( r.height() );

    return;
}

// ROI area 4 changed
void QEImage::roi4Changed()
{
    // Write the ROI variables.
    QEInteger *qca;
    QPoint p1 = iProcessor.rotateFlipToDataPoint( selectedArea4Point1 );
    QPoint p2 = iProcessor.rotateFlipToDataPoint( selectedArea4Point2 );
    QRect r( p1, p2 );
    r = r.normalized();

    qca = (QEInteger*)getQcaItem( ROI4_X_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().x() );

    qca = (QEInteger*)getQcaItem( ROI4_Y_VARIABLE );
    if( qca ) qca->writeInteger( r.topLeft().y() );

    qca = (QEInteger*)getQcaItem( ROI4_W_VARIABLE );
    if( qca ) qca->writeInteger( r.width() );

    qca = (QEInteger*)getQcaItem( ROI4_H_VARIABLE );
    if( qca ) qca->writeInteger( r.height() );

    return;
}

// Arbitrary line profile  changed
void QEImage::lineProfileChanged()
{
    // Write the arbitrary line profile variables.
    QEInteger *qca;
    QPoint p1 = iProcessor.rotateFlipToDataPoint( profileLineStart );
    QPoint p2 = iProcessor.rotateFlipToDataPoint( profileLineEnd );

    qca = (QEInteger*)getQcaItem( LINE_PROFILE_X1_VARIABLE );
    if( qca ) qca->writeInteger( p1.x() );

    qca = (QEInteger*)getQcaItem( LINE_PROFILE_Y1_VARIABLE );
    if( qca ) qca->writeInteger( p1.y() );

    qca = (QEInteger*)getQcaItem( LINE_PROFILE_X2_VARIABLE );
    if( qca ) qca->writeInteger( p2.x() );

    qca = (QEInteger*)getQcaItem( LINE_PROFILE_Y2_VARIABLE );
    if( qca ) qca->writeInteger( p2.y() );

    qca = (QEInteger*)getQcaItem( LINE_PROFILE_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( profileThickness );

    return;
}

// Horizontal line profile 1 changed
void QEImage::hozProfile1Changed()
{
    // Write the horizontal line 1 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_H1_VARIABLE );
    if( qca ) qca->writeInteger( hSlice1Y );

    qca = (QEInteger*)getQcaItem( PROFILE_H1_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( hSlice1Thickness );

    return;
}

// Horizontal line profile 2 changed
void QEImage::hozProfile2Changed()
{
    // Write the horizontal line 2 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_H2_VARIABLE );
    if( qca ) qca->writeInteger( hSlice2Y );

    qca = (QEInteger*)getQcaItem( PROFILE_H2_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( hSlice2Thickness );

    return;
}

// Horizontal line profile 3 changed
void QEImage::hozProfile3Changed()
{
    // Write the horizontal line 3 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_H3_VARIABLE );
    if( qca ) qca->writeInteger( hSlice3Y );

    qca = (QEInteger*)getQcaItem( PROFILE_H3_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( hSlice3Thickness );

    return;
}

// Horizontal line profile 4 changed
void QEImage::hozProfile4Changed()
{
    // Write the horizontal line 4 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_H4_VARIABLE );
    if( qca ) qca->writeInteger( hSlice4Y );

    qca = (QEInteger*)getQcaItem( PROFILE_H4_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( hSlice4Thickness );

    return;
}

// Horizontal line profile 5 changed
void QEImage::hozProfile5Changed()
{
    // Write the horizontal line 4 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_H5_VARIABLE );
    if( qca ) qca->writeInteger( hSlice5Y );

    qca = (QEInteger*)getQcaItem( PROFILE_H5_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( hSlice5Thickness );

    return;
}

// Vertical line profile 1 changed
void QEImage::vertProfile1Changed()
{
    // Write the vertical line profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_V1_VARIABLE );
    if( qca ) qca->writeInteger( vSlice1X );

    qca = (QEInteger*)getQcaItem( PROFILE_V1_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( vSlice1Thickness );

    return;
}

// Vertical line profile 2 changed
void QEImage::vertProfile2Changed()
{
    // Write the vertical line 2 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_V2_VARIABLE );
    if( qca ) qca->writeInteger( vSlice2X );

    qca = (QEInteger*)getQcaItem( PROFILE_V2_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( vSlice2Thickness );

    return;
}

// Vertical line profile 3 changed
void QEImage::vertProfile3Changed()
{
    // Write the vertical line 3 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_V3_VARIABLE );
    if( qca ) qca->writeInteger( vSlice3X );

    qca = (QEInteger*)getQcaItem( PROFILE_V3_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( vSlice3Thickness );

    return;
}

// Vertical line profile 4 changed
void QEImage::vertProfile4Changed()
{
    // Write the vertical line 4 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_V4_VARIABLE );
    if( qca ) qca->writeInteger( vSlice4X );

    qca = (QEInteger*)getQcaItem( PROFILE_V4_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( vSlice4Thickness );

    return;
}

// Vertical line profile 5 changed
void QEImage::vertProfile5Changed()
{
    // Write the vertical line 5 profile variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( PROFILE_V5_VARIABLE );
    if( qca ) qca->writeInteger( vSlice5X );

    qca = (QEInteger*)getQcaItem( PROFILE_V5_THICKNESS_VARIABLE );
    if( qca ) qca->writeInteger( vSlice5Thickness );

    return;
}

// Move target into beam button pressed
void QEImage::targetClicked()
{
    // Write to the target trigger variable.
    QEInteger *qca;
    qca = (QEInteger*)getQcaItem( TARGET_TRIGGER_VARIABLE );
    if( qca ) qca->writeInteger( 1 );

    return;
}

// Pause button pressed
void QEImage::pauseClicked()
{
    // If paused, resume
    if (paused)
    {
        // Resume display of most recent image
        pauseButton->setIcon( *pauseButtonIcon );
        pauseButton->setToolTip("Pause image display");
        paused = false;
    }

    // Not paused, so pause
    else
    {
        // Pause the display
        pauseButton->setIcon( *playButtonIcon );
        pauseButton->setToolTip("Resume image display");
        paused = true;
    }

    // Ensure the checked state of the internal pause buttons is set correctly.
    // (A change in pause state may have originated from the external QAction)
    pauseButton->setChecked( paused );

    // If there is an external QAction that can pause display, set its checked state correctly.
    // (A change in pause state may have originated from the interbal button )
    if( pauseExternalAction )
    {
        pauseExternalAction->setChecked( paused );
    }

    // Update the info area
    infoUpdatePaused( paused );

}

// Save button pressed
void QEImage::saveClicked()
{
    QFileDialog *qFileDialog;
    QStringList filterList;
    QString filename;
    bool result;

    qFileDialog = new QFileDialog(this, "Save displayed image", QDir::currentPath().append( QDir::separator() ).append("image.png") );
    filterList << "Tagged Image File Format (*.tiff)" << "Portable Network Graphics (*.png)" << "Windows Bitmap (*.bmp)" << "Joint Photographics Experts Group (*.jpg)";
    qFileDialog->setNameFilters(filterList);
    qFileDialog->setAcceptMode(QFileDialog::AcceptSave);

// Don't set default suffix since the filename as entered is checked for existance (and
// replacement confirmed with the user), then the filename with suffix is returned!
// this means a file may be overwritten without warning, or warning may be given,
// then a different file created
//    qFileDialog->setDefaultSuffix( "png" );

// Don't avoid native dialog as they are much richer.
//    qFileDialog->setOption ( QFileDialog::DontUseNativeDialog, true );

    if (qFileDialog->exec())
    {
        QImage qImage = iProcessor.copyImage();
        filename = qFileDialog->selectedFiles().at(0);

        if (qFileDialog->selectedNameFilter() == filterList.at(0))
        {
            result = qImage.save(filename, "TIFF");
        }
        else if (qFileDialog->selectedNameFilter() == filterList.at(1))
        {
            result = qImage.save(filename, "PNG");
        }
        else if (qFileDialog->selectedNameFilter() == filterList.at(2))
        {
            result = qImage.save(filename, "BMP");
        }
        else
        {
            result = qImage.save(filename, "JPG");
        }

        if (result)
        {
            QMessageBox::information(this, "Info", "The displayed image was successfully saved in file '" + filename + "'!");
        }
        else
        {
            QMessageBox::critical(this, "Error", "Unable to save displayed image in file '" + filename + "'!");
        }
    }

}

// Update the video widget if the QEImage has changed
void QEImage::resizeEvent( QResizeEvent* )
{
    setImageSize();
    displayImage();
}


//==============================================================================

void QEImage::doEnableButtonBar( bool enableButtonBar )
{
    buttonGroup->setVisible( enableButtonBar );
}

// Manage image display properties controls such as brightness and contrast
void QEImage::doEnableImageDisplayProperties( bool enableImageDisplayProperties )
{
    if( !imageDisplayProps )
    {
        return;
    }

    imageDisplayProps->setVisible( enableImageDisplayProperties );
}

// Manage image display properties controls such as brightness and contrast
void QEImage::doEnableRecording( bool enableRecording )
{
    if( !recorder )
    {
        return;
    }

    recorder->setVisible( enableRecording );
}

// Manage contrast reversal
void QEImage::doContrastReversal( bool /*contrastReversal*/ )
{
    // Flag color lookup table is invalid
    iProcessor.invalidatePixelLookup();

    // Redraw the current image (don't wait for next update (image may be stalled)
    redraw();
}

// Manage vertical slice 1 selection
void QEImage::doEnableVertSlice1Selection( bool enableVSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_VSLICE1, enableVSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_VSLICE1, enableVSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableVSliceSelection )
    {
        if( getSelectionOption() == SO_VSLICE1 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_V1_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE1 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_V1_SLICE );
        }
    }
}

// Manage vertical slice 2 selection
void QEImage::doEnableVertSlice2Selection( bool enableVSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_VSLICE2, enableVSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_VSLICE2, enableVSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableVSliceSelection )
    {
        if( getSelectionOption() == SO_VSLICE2 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_V2_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE2 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_V2_SLICE );
        }
    }
}

// Manage vertical slice 3 selection
void QEImage::doEnableVertSlice3Selection( bool enableVSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_VSLICE3, enableVSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_VSLICE3, enableVSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableVSliceSelection )
    {
        if( getSelectionOption() == SO_VSLICE3 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE3 ) )
        {
            videoWidget->clearMarkup( imageMarkup::MARKUP_ID_V3_SLICE );
        }
    }
    else
    {
        videoWidget->showMarkup( imageMarkup::MARKUP_ID_V3_SLICE );
    }
}

// Manage vertical slice 4 selection
void QEImage::doEnableVertSlice4Selection( bool enableVSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_VSLICE4, enableVSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_VSLICE4, enableVSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableVSliceSelection )
    {
        if( getSelectionOption() == SO_VSLICE4 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_V4_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE4 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_V4_SLICE );
        }
    }
}

// Manage vertical slice selection
void QEImage::doEnableVertSlice5Selection( bool enableVSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_VSLICE5, enableVSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_VSLICE5, enableVSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableVSliceSelection )
    {
        if( getSelectionOption() == SO_VSLICE5 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_V5_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE5 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_V5_SLICE );
        }
    }
}

// Enable horizontal slice 1 selection
void QEImage::doEnableHozSlice1Selection( bool enableHSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_HSLICE1, enableHSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_HSLICE1, enableHSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableHSliceSelection )
    {
        if( getSelectionOption() == SO_HSLICE1 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_H1_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE1 ) )
        {
           videoWidget->showMarkup( imageMarkup::MARKUP_ID_H1_SLICE );
        }
    }
}

// Enable horizontal slice 2 selection
void QEImage::doEnableHozSlice2Selection( bool enableHSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_HSLICE2, enableHSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_HSLICE2, enableHSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableHSliceSelection )
    {
        if( getSelectionOption() == SO_HSLICE2 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_H2_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE2 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_H2_SLICE );
        }
    }
}

// Enable horizontal slice 3 selection
void QEImage::doEnableHozSlice3Selection( bool enableHSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_HSLICE3, enableHSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_HSLICE3, enableHSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableHSliceSelection )
    {
        if( getSelectionOption() == SO_HSLICE3 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_H3_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE3 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_H3_SLICE );
        }
    }
}

// Enable horizontal slice 4 selection
void QEImage::doEnableHozSlice4Selection( bool enableHSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_HSLICE4, enableHSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_HSLICE4, enableHSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableHSliceSelection )
    {
        if( getSelectionOption() == SO_HSLICE4 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_H4_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE4 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_H4_SLICE );
        }
    }
}

// Enable horizontal slice 5 selection
void QEImage::doEnableHozSlice5Selection( bool enableHSliceSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_HSLICE5, enableHSliceSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_HSLICE5, enableHSliceSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableHSliceSelection )
    {
        if( getSelectionOption() == SO_HSLICE5 )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_H5_SLICE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE5 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_H5_SLICE );
        }
    }
}

// Enable area selection (used for ROI and zoom)
void QEImage::doEnableAreaSelection( /*imageContextMenu::imageContextMenuOptions area,*/ bool enableAreaSelection )
{
    // Ensure the area selection menus are set up correctly
    sMenu->enable( imageContextMenu::ICM_SELECT_AREA1, enableAreaSelection );
    sMenu->enable( imageContextMenu::ICM_SELECT_AREA2, enableAreaSelection );
    sMenu->enable( imageContextMenu::ICM_SELECT_AREA3, enableAreaSelection );
    sMenu->enable( imageContextMenu::ICM_SELECT_AREA4, enableAreaSelection );

    mdMenu->enable( imageContextMenu::ICM_DISPLAY_AREA1, enableAreaSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_AREA2, enableAreaSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_AREA3, enableAreaSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_AREA4, enableAreaSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableAreaSelection )
    {
        if( ( ( getSelectionOption() == SO_AREA1 ) ||
              ( getSelectionOption() == SO_AREA2 ) ||
              ( getSelectionOption() == SO_AREA3 ) ||
              ( getSelectionOption() == SO_AREA4 )))
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_REGION1 );
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_REGION2 );
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_REGION3 );
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_REGION4 );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_AREA1 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_REGION1 );
        }
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_AREA2 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_REGION2 );
        }
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_AREA3 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_REGION3 );
        }
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_AREA4 ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_REGION4 );
        }
    }
}

// Manage profile selection
void QEImage::doEnableProfileSelection( bool enableProfileSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_PROFILE, enableProfileSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_PROFILE, enableProfileSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableProfileSelection )
    {
        if( getSelectionOption() == SO_PROFILE )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_LINE );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_PROFILE ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_LINE );
        }
    }
}

// Manage target selection
void QEImage::doEnableTargetSelection( bool enableTargetSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_TARGET, enableTargetSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_TARGET, enableTargetSelection );

    targetButton->setVisible( enableTargetSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableTargetSelection )
    {
        if( getSelectionOption() == SO_TARGET )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_TARGET );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_TARGET ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_TARGET );
        }
    }
}

// Manage beam selection
void QEImage::doEnableBeamSelection( bool enableBeamSelection )
{
    sMenu->enable( imageContextMenu::ICM_SELECT_BEAM, enableBeamSelection );
    mdMenu->enable( imageContextMenu::ICM_DISPLAY_BEAM, enableBeamSelection );

    // If disabling, and it is the current mode, then default to panning
    if( !enableBeamSelection )
    {
        if( getSelectionOption() == SO_BEAM )
        {
            sMenu->setChecked( QEImage::SO_PANNING );
            panModeClicked();
        }
        videoWidget->clearMarkup( imageMarkup::MARKUP_ID_BEAM );
    }
    else
    {
        if( mdMenu->isDisplayed( imageContextMenu::ICM_DISPLAY_BEAM ) )
        {
            videoWidget->showMarkup( imageMarkup::MARKUP_ID_BEAM );
        }
    }
}

//==============================================================================
// Drag drop
void QEImage::setDrop( QVariant drop )
{
    setVariableName( drop.toString(), 0 );
    establishConnection( 0 );
}

QVariant QEImage::getDrop()
{
    if( isDraggingVariable() )
        return QVariant( copyVariable() );
    else
        return copyData();
}

//==============================================================================
// Copy / Paste
QString QEImage::copyVariable()
{
    return getSubstitutedVariableName(0);
}

QVariant QEImage::copyData()
{
    return QVariant( videoWidget->getImage() );
}

void QEImage::paste( QVariant v )
{
    if( getAllowDrop() )
    {
        setDrop( v );
    }
}

//==============================================================================
// Property convenience functions


// Allow user to set the video format
void QEImage::setFormatOption( imageDataFormats::formatOptions formatOptionIn )
{
    iProcessor.setFormat( formatOptionIn );
}

imageDataFormats::formatOptions QEImage::getFormatOption()
{
    return iProcessor.getFormat();
}

// Allow user to set the bit depth for Mono video format
void QEImage::setBitDepth( unsigned int bitDepthIn )
{
    // Ensure bit depth is reasonable
    unsigned int sanitiedBitDepth = bitDepthIn;
    if( sanitiedBitDepth == 0 )
    {
        sanitiedBitDepth = 1;
    }
    else if( sanitiedBitDepth > 32 )
    {
        sanitiedBitDepth = 32;
    }

    // Save the option
    iProcessor.setBitDepth( sanitiedBitDepth );
}

unsigned int QEImage::getBitDepth()
{
    return iProcessor.getBitDepth();
}

// Set the zoom percentage (and force zoom mode)
void QEImage::setResizeOptionAndZoom( int zoomIn )
{

    // !!! do each of the following two lines call setImageBuff()???
    setResizeOption( RESIZE_OPTION_ZOOM );
    setZoom( zoomIn );
}

// Zoom level
void QEImage::setZoom( int zoomIn )
{
    // Save the zoom
    // (Limit to 10 - 400 %)
    if( zoomIn < 10 )
        zoom = 10;
    else if( zoomIn > 400 )
        zoom = 400;
    else
        zoom = zoomIn;

    // Resize and rescale
    setImageSize();

    // Update the info area
    infoUpdateZoom( zoom, XStretch, YStretch );

}

int QEImage::getZoom()
{
    return zoom;
}

// X stretch factor. Used when determining canvas size of fully processed image (zoomed, flipped, etc)

#define STRETCH_LIMIT 50

void QEImage::setXStretch( double XStretchIn )
{
    // Save the X stretch factor
    // (Limit to a range of STRETCH_LIMIT times)
    if( XStretchIn < (1/STRETCH_LIMIT) )
        XStretch = 1/STRETCH_LIMIT;
    else if( XStretchIn > STRETCH_LIMIT )
        XStretch = STRETCH_LIMIT;
    else
        XStretch = XStretchIn;

    // Resize and rescale
    setImageSize();

    // Update the info area
    infoUpdateZoom( zoom, XStretch, YStretch );

}

double QEImage::getXStretch()
{
    return XStretch;
}

// Y stretch factor. Used when determining canvas size of fully processed image (zoomed, flipped, etc)

void QEImage::setYStretch( double YStretchIn )
{
    // Save the Y stretch factor
    // (Limit to a range of STRETCH_LIMIT times)
    if( YStretchIn < (1/STRETCH_LIMIT) )
        YStretch = 1/STRETCH_LIMIT;
    else if( YStretchIn > STRETCH_LIMIT )
        YStretch = STRETCH_LIMIT;
    else
        YStretch = YStretchIn;

    // Resize and rescale
    setImageSize();

    // Update the info area
    infoUpdateZoom( zoom, XStretch, YStretch );

}

double QEImage::getYStretch()
{
    return YStretch;
}

// Rotation
void QEImage::setRotation( imageProperties::rotationOptions rotationIn )
{
    // Save the rotation requested
    iProcessor.setRotation( rotationIn );

    // Adjust the size of the image to maintain aspect ratio if required
    setImageSize();

    // Present the updated image
    displayImage();
    redisplayAllMarkups();
}

imageProperties::rotationOptions QEImage::getRotation()
{
    return iProcessor.getRotation();
}

// Horizontal flip
void QEImage::setHorizontalFlip( bool flipHozIn )
{
    iProcessor.setFlipHoz( flipHozIn );

    // Present the updated image
    displayImage();
    redisplayAllMarkups();
}

bool QEImage::getHorizontalFlip()
{
    return iProcessor.getFlipHoz();
}

// Vertical flip
void QEImage::setVerticalFlip( bool flipVertIn )
{
    iProcessor.setFlipVert( flipVertIn );

    // Present the updated image
    displayImage();
    redisplayAllMarkups();
}

bool QEImage::getVerticalFlip()
{
    return iProcessor.getFlipVert();
}

// Automatic setting of brightness and contrast on region selection
void QEImage::setAutoBrightnessContrast( bool autoBrightnessContrastIn )
{
    if( !imageDisplayProps )
    {
        return;
    }

    imageDisplayProps->setAutoBrightnessContrast( autoBrightnessContrastIn );
}

bool QEImage::getAutoBrightnessContrast()
{
    if( !imageDisplayProps )
    {
        return false;
    }
    return imageDisplayProps->getAutoBrightnessContrast();
}

// Resize options
void QEImage::setResizeOption( resizeOptions resizeOptionIn )
{
    // Save the resize option
    resizeOption = resizeOptionIn;

    // Resize and rescale
    setImageSize();

    // Present the updated image
    displayImage();
}

QEImage::resizeOptions QEImage::getResizeOption()
{
    return resizeOption;
}

// Initial vorizontal scroll position
void QEImage::setInitialHozScrollPos( int initialHozScrollPosIn )
{
    // Save the position
    initialHozScrollPos = initialHozScrollPosIn;

    // Set the position.
    // Note, will work when changed interactivly in designer. When the property is loaded
    // on startup, setting the scoll bars won't work until the widget has been presented
    // and geometry is sorted out. that will happen when the image is first displayed
    scrollArea->horizontalScrollBar()->setValue( initialHozScrollPos );
}

int QEImage::getInitialHozScrollPos()
{
    return initialHozScrollPos;
}

// Initial vertical scroll position
void QEImage::setInitialVertScrollPos( int initialVertScrollPosIn )
{
    // Save the position
    initialVertScrollPos = initialVertScrollPosIn;

    // Set the position.
    // Note, will work when changed interactivly in designer. When the property is loaded
    // on startup, setting the scoll bars won't work until the widget has been presented
    // and geometry is sorted out. that will happen when the image is first displayed
    scrollArea->verticalScrollBar()->setValue( initialVertScrollPos );
}

int QEImage::getInitialVertScrollPos()
{
    return initialVertScrollPos;
}

// Show time
void QEImage::setShowTime(bool value)
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_TIME, value );
}

bool QEImage::getShowTime()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_TIME );
}

// Use False Colour
void QEImage::setUseFalseColour(bool value)
{
    imageDisplayProps->setFalseColour( value );
}

bool QEImage::getUseFalseColour()
{
    return imageDisplayProps->getFalseColour();
}

// Vertical slice 1 markup colour
void QEImage::setVertSlice1MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_V1_SLICE, markupColor );
}

QColor QEImage::getVertSlice1MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_V1_SLICE );
}

// Vertical slice 2markup colour
void QEImage::setVertSlice2MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_V2_SLICE, markupColor );
}

QColor QEImage::getVertSlice2MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_V2_SLICE );
}

// Vertical slice 3 markup colour
void QEImage::setVertSlice3MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_V3_SLICE, markupColor );
}

QColor QEImage::getVertSlice3MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_V3_SLICE );
}

// Vertical slice 4 markup colour
void QEImage::setVertSlice4MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_V4_SLICE, markupColor );
}

QColor QEImage::getVertSlice4MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_V4_SLICE );
}

// Vertical slice 5 markup colour
void QEImage::setVertSlice5MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_V5_SLICE, markupColor );
}

QColor QEImage::getVertSlice5MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_V5_SLICE );
}

// Horizontal slice 1 markup colour
void QEImage::setHozSlice1MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_H1_SLICE, markupColor );
}

QColor QEImage::getHozSlice1MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_H1_SLICE );
}

// Horizontal slice 2 markup colour
void QEImage::setHozSlice2MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_H2_SLICE, markupColor );
}

QColor QEImage::getHozSlice2MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_H2_SLICE );
}

// Horizontal slice 3 markup colour
void QEImage::setHozSlice3MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_H3_SLICE, markupColor );
}

QColor QEImage::getHozSlice3MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_H3_SLICE );
}

// Horizontal slice 4 markup colour
void QEImage::setHozSlice4MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_H4_SLICE, markupColor );
}

QColor QEImage::getHozSlice4MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_H4_SLICE );
}

// Horizontal slice 5 markup colour
void QEImage::setHozSlice5MarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_H5_SLICE, markupColor );
}

QColor QEImage::getHozSlice5MarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_H5_SLICE );
}

// Profile markup colour
void QEImage::setProfileMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_LINE, markupColor );
}

QColor QEImage::getProfileMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_LINE );
}

// Area markup colour
void QEImage::setAreaMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_REGION1, markupColor );
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_REGION2, markupColor );
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_REGION3, markupColor );
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_REGION4, markupColor );
}

QColor QEImage::getAreaMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_REGION1 ); // same as MARKUP_ID_REGION2, MARKUP_ID_REGION3 and MARKUP_ID_REGION4
}

// Area markup colour
void QEImage::setTimeMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_TIMESTAMP, markupColor );
}

QColor QEImage::getTimeMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_TIMESTAMP );
}

// Target slice markup colour
void QEImage::setTargetMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_TARGET, markupColor );
}

QColor QEImage::getTargetMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_TARGET );
}

// Beam slice markup colour
void QEImage::setBeamMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_BEAM, markupColor );
}

QColor QEImage::getBeamMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_BEAM );
}

// Ellipse markup colour
void QEImage::setEllipseMarkupColor(QColor markupColor )
{
    videoWidget->setMarkupColor( imageMarkup::MARKUP_ID_ELLIPSE, markupColor );
}

QColor QEImage::getEllipseMarkupColor()
{
    return videoWidget->getMarkupColor( imageMarkup::MARKUP_ID_ELLIPSE );
}

// Display the button bar
void QEImage::setDisplayButtonBar( bool displayButtonBar )
{
    optionsDialog->optionSet( imageContextMenu::ICM_DISPLAY_BUTTON_BAR, displayButtonBar );
}

bool QEImage::getDisplayButtonBar()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_DISPLAY_BUTTON_BAR );
}

// Show cursor pixel
void QEImage::setDisplayCursorPixelInfo( bool displayCursorPixelInfo )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL, displayCursorPixelInfo );
}

bool QEImage::getDisplayCursorPixelInfo()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL );
}

// Show contrast reversal
void QEImage::setContrastReversal( bool contrastReversal )
{
    imageDisplayProps->setContrastReversal( contrastReversal );
}

bool QEImage::getContrastReversal()
{
    return imageDisplayProps->getContrastReversal();
}

// Show log brightness scale
void QEImage::setLog( bool log )
{
    imageDisplayProps->setLog( log );
}

bool QEImage::getLog()
{
    return imageDisplayProps->getLog();
}

// Enable vertical slice 1 selection
void QEImage::setEnableVertSlice1Selection( bool enableVSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_VERT1, enableVSliceSelection );
}

bool QEImage::getEnableVertSlice1Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_VERT1 );
}

// Enable vertical slice 2 selection
void QEImage::setEnableVertSlice2Selection( bool enableVSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_VERT2, enableVSliceSelection );
}

bool QEImage::getEnableVertSlice2Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_VERT2 );
}

// Enable vertical slice 3 selection
void QEImage::setEnableVertSlice3Selection( bool enableVSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_VERT3, enableVSliceSelection );
}

bool QEImage::getEnableVertSlice3Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_VERT3 );
}

// Enable vertical slice 4 selection
void QEImage::setEnableVertSlice4Selection( bool enableVSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_VERT4, enableVSliceSelection );
}

bool QEImage::getEnableVertSlice4Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_VERT4 );
}

// Enable vertical slice 5 selection
void QEImage::setEnableVertSlice5Selection( bool enableVSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_VERT5, enableVSliceSelection );
}

bool QEImage::getEnableVertSlice5Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_VERT5 );
}

// Enable horizontal slice 1 selection
void QEImage::setEnableHozSlice1Selection( bool enableHSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_HOZ1, enableHSliceSelection );
}

bool QEImage::getEnableHozSlice1Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_HOZ1 );
}

// Enable horizontal slice 2 selection
void QEImage::setEnableHozSlice2Selection( bool enableHSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_HOZ2, enableHSliceSelection );
}

bool QEImage::getEnableHozSlice2Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_HOZ2 );
}

// Enable horizontal slice 3 selection
void QEImage::setEnableHozSlice3Selection( bool enableHSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_HOZ3, enableHSliceSelection );
}

bool QEImage::getEnableHozSlice3Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_HOZ3 );
}

// Enable horizontal slice 4 selection
void QEImage::setEnableHozSlice4Selection( bool enableHSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_HOZ4, enableHSliceSelection );
}

bool QEImage::getEnableHozSlice4Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_HOZ4 );
}

// Enable horizontal slice 5 selection
void QEImage::setEnableHozSlice5Selection( bool enableHSliceSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_HOZ4, enableHSliceSelection );
}

bool QEImage::getEnableHozSlice5Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_HOZ4 );
}

// Enable area 1 selection (used for ROI and zoom)
void QEImage::setEnableArea1Selection( bool enableAreaSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_AREA1, enableAreaSelection );
}

bool QEImage::getEnableArea1Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_AREA1 );
}

// Enable area 2 selection (used for ROI and zoom)
void QEImage::setEnableArea2Selection( bool enableAreaSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_AREA2, enableAreaSelection );
}

bool QEImage::getEnableArea2Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_AREA2 );
}

// Enable area 3 selection (used for ROI and zoom)
void QEImage::setEnableArea3Selection( bool enableAreaSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_AREA3, enableAreaSelection );
}

bool QEImage::getEnableArea3Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_AREA3 );
}

// Enable area 4 selection (used for ROI and zoom)
void QEImage::setEnableArea4Selection( bool enableAreaSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_AREA4, enableAreaSelection );
}

bool QEImage::getEnableArea4Selection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_AREA4 );
}

// Enable profile selection
void QEImage::setEnableProfileSelection( bool enableProfileSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_LINE, enableProfileSelection );
}

bool QEImage::getEnableProfileSelection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_LINE );
}

// Enable target selection
void QEImage::setEnableTargetSelection( bool enableTargetSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_TARGET, enableTargetSelection );
}

bool QEImage::getEnableTargetSelection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_TARGET );
}

// Enable beam selection
void QEImage::setEnableBeamSelection( bool enableBeamSelection )
{
    optionsDialog->optionSet( imageContextMenu::ICM_ENABLE_BEAM, enableBeamSelection );
}

bool QEImage::getEnableBeamSelection()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_ENABLE_BEAM );
}

//=====================


// Enable profile presentation
void QEImage::setEnableProfilePresentation( bool enableProfilePresentationIn )
{
    enableProfilePresentation = enableProfilePresentationIn;
}

bool QEImage::getEnableProfilePresentation()
{
    return enableProfilePresentation;
}

// Enable horizontal profile presentation
void QEImage::setEnableHozSlicePresentation( bool enableHozSlicePresentationIn )
{
    enableHozSlicePresentation = enableHozSlicePresentationIn;
}

bool QEImage::getEnableHozSlicePresentation()
{
    return enableHozSlicePresentation;
}

// Enable vertical profile presentation
void QEImage::setEnableVertSlicePresentation( bool enableVertSlicePresentationIn )
{
    enableVertSlicePresentation = enableVertSlicePresentationIn;
}

bool QEImage::getEnableVertSlicePresentation()
{
    return enableVertSlicePresentation;
}

//=====================

// Display vertical slice 1 selection
void QEImage::setDisplayVertSlice1Selection( bool displayVSliceSelection )
{

    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V1_SLICE, displayVSliceSelection );
}

bool QEImage::getDisplayVertSlice1Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_V1_SLICE );
}

// Display vertical slice 2 selection
void QEImage::setDisplayVertSlice2Selection( bool displayVSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V2_SLICE, displayVSliceSelection );
}

bool QEImage::getDisplayVertSlice2Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_V2_SLICE );
}

// Display vertical slice 3 selection
void QEImage::setDisplayVertSlice3Selection( bool displayVSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V3_SLICE, displayVSliceSelection );
}

bool QEImage::getDisplayVertSlice3Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_V3_SLICE );
}

// Display vertical slice 4 selection
void QEImage::setDisplayVertSlice4Selection( bool displayVSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V4_SLICE, displayVSliceSelection );
}

bool QEImage::getDisplayVertSlice4Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_V4_SLICE );
}

// Display vertical slice 5 selection
void QEImage::setDisplayVertSlice5Selection( bool displayVSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V5_SLICE, displayVSliceSelection );
}

bool QEImage::getDisplayVertSlice5Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_V5_SLICE );
}

// Display horizontal slice 1 selection
void QEImage::setDisplayHozSlice1Selection( bool displayHSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H1_SLICE, displayHSliceSelection );
}

bool QEImage::getDisplayHozSlice1Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_H1_SLICE );
}

// Display horizontal slice 2 selection
void QEImage::setDisplayHozSlice2Selection( bool displayHSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H2_SLICE, displayHSliceSelection );
}

bool QEImage::getDisplayHozSlice2Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_H2_SLICE );
}

// Display horizontal slice 3 selection
void QEImage::setDisplayHozSlice3Selection( bool displayHSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H3_SLICE, displayHSliceSelection );
}

bool QEImage::getDisplayHozSlice3Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_H3_SLICE );
}

// Display horizontal slice 4 selection
void QEImage::setDisplayHozSlice4Selection( bool displayHSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H4_SLICE, displayHSliceSelection );
}

bool QEImage::getDisplayHozSlice4Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_H4_SLICE );
}

// Display horizontal slice 5 selection
void QEImage::setDisplayHozSlice5Selection( bool displayHSliceSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H5_SLICE, displayHSliceSelection );
}

bool QEImage::getDisplayHozSlice5Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_H5_SLICE );
}

// Display area 1 selection (used for ROI and zoom)
void QEImage::setDisplayArea1Selection( bool displayAreaSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION1, displayAreaSelection );
}

bool QEImage::getDisplayArea1Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_REGION1 );
}

// Display area 2 selection (used for ROI and zoom)
void QEImage::setDisplayArea2Selection( bool displayAreaSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION2, displayAreaSelection );
}

bool QEImage::getDisplayArea2Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_REGION2 );
}

// Display area 3 selection (used for ROI and zoom)
void QEImage::setDisplayArea3Selection( bool displayAreaSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION3, displayAreaSelection );
}

bool QEImage::getDisplayArea3Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_REGION3 );
}

// Display area 4 selection (used for ROI and zoom)
void QEImage::setDisplayArea4Selection( bool displayAreaSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION4, displayAreaSelection );
}

bool QEImage::getDisplayArea4Selection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_REGION4 );
}

// Display profile selection
void QEImage::setDisplayProfileSelection( bool displayProfileSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_LINE, displayProfileSelection );
}

bool QEImage::getDisplayProfileSelection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_LINE );
}

// Display target selection
void QEImage::setDisplayTargetSelection( bool displayTargetSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_TARGET, displayTargetSelection );
}

bool QEImage::getDisplayTargetSelection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_TARGET );
}

// Display beam selection
void QEImage::setDisplayBeamSelection( bool displayBeamSelection )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_BEAM, displayBeamSelection);
}

bool QEImage::getDisplayBeamSelection()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_BEAM );
}

// Display ellipse
void QEImage::setDisplayEllipse( bool displayEllipse )
{
    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_ELLIPSE, displayEllipse );
}

bool QEImage::getDisplayEllipse()
{
    return videoWidget->isMarkupVisible( imageMarkup::MARKUP_ID_ELLIPSE );
}

// Ellipse variable usage
void QEImage::setEllipseVariableDefinition( ellipseVariableDefinitions variableUsage )
{
    ellipseVariableUsage = variableUsage;
}

QEImage::ellipseVariableDefinitions QEImage::getEllipseVariableDefinition()
{
    return ellipseVariableUsage;
}

//videoWidget->displayMarkup( imageMarkup::MARKUP_ID_TIMESTAMP, selectedItem->isChecked() );

//==================

// Enable local brightness and contrast controls if required
void QEImage::setEnableImageDisplayProperties( bool enableImageDisplayProperties )
{
    optionsDialog->optionSet( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES, enableImageDisplayProperties );
}

bool QEImage::getEnableImageDisplayProperties()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES );
}

// Enable recording and playback
void QEImage::setEnableRecording( bool enableRecording )
{
    optionsDialog->optionSet( imageContextMenu::ICM_DISPLAY_RECORDER, enableRecording );
}

bool QEImage::getEnableRecording()
{
    return optionsDialog->optionGet( imageContextMenu::ICM_DISPLAY_RECORDER );
}

// Request the application host controls such as toolbars and profile views for this widget
void QEImage::setExternalControls( bool externalControlsIn )
{
    appHostsControls = externalControlsIn;
    presentControls();
}

bool QEImage::getExternalControls()
{
    return appHostsControls;
}

// Determine if a full context menu allowing manipulation of the image is available or not
void QEImage::setFullContextMenu( bool fullContextMenuIn )
{
    fullContextMenu = fullContextMenuIn;
    return;
}

bool QEImage::getFullContextMenu()
{
    return fullContextMenu;
}

// Display all markups for which there is data available.
void QEImage::setDisplayMarkups( bool displayMarkupsIn )
{
    displayMarkups = displayMarkupsIn;
}

bool QEImage::getDisplayMarkups()
{
    return displayMarkups;
}


// Name of widget for display and identification purpose
void QEImage::setName( QString nameIn )
{
    name = nameIn;
}

QString QEImage::getName()
{
    return name;
}

// Application launching
// Program String
void QEImage::setProgram1( QString program ){ programLauncher1.setProgram( program ); }
QString QEImage::getProgram1(){ return programLauncher1.getProgram(); }
void QEImage::setProgram2( QString program ){ programLauncher2.setProgram( program ); }
QString QEImage::getProgram2(){ return programLauncher2.getProgram(); }

// Arguments String
void QEImage::setArguments1( QStringList arguments ){ programLauncher1.setArguments( arguments ); }
QStringList QEImage::getArguments1(){ return  programLauncher1.getArguments(); }
void QEImage::setArguments2( QStringList arguments ){ programLauncher2.setArguments( arguments ); }
QStringList QEImage::getArguments2(){ return  programLauncher2.getArguments(); }

// Startup option
void QEImage::setProgramStartupOption1( applicationLauncher::programStartupOptions programStartupOption ){ programLauncher1.setProgramStartupOption( programStartupOption ); }
applicationLauncher::programStartupOptions QEImage::getProgramStartupOption1(){ return programLauncher1.getProgramStartupOption(); }
void QEImage::setProgramStartupOption2( applicationLauncher::programStartupOptions programStartupOption ){ programLauncher2.setProgramStartupOption( programStartupOption ); }
applicationLauncher::programStartupOptions QEImage::getProgramStartupOption2(){ return programLauncher2.getProgramStartupOption(); }

// Legends
QString QEImage::getHozSlice1Legend()                      { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_H1_SLICE );        }
void    QEImage::setHozSlice1Legend      ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_H1_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_HSLICE1, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_HSLICE1, legend ); }
QString QEImage::getHozSlice2Legend()                      { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_H2_SLICE );        }
void    QEImage::setHozSlice2Legend      ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_H2_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_HSLICE2, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_HSLICE2, legend ); }
QString QEImage::getHozSlice3Legend()                      { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_H3_SLICE );        }
void    QEImage::setHozSlice3Legend      ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_H3_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_HSLICE3, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_HSLICE3, legend ); }
QString QEImage::getHozSlice4Legend()                      { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_H4_SLICE );        }
void    QEImage::setHozSlice4Legend      ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_H4_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_HSLICE4, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_HSLICE4, legend ); }
QString QEImage::getHozSlice5Legend()                      { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_H5_SLICE );        }
void    QEImage::setHozSlice5Legend      ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_H5_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_HSLICE5, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_HSLICE5, legend ); }
QString QEImage::getVertSlice1Legend()                     { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_V1_SLICE );        }
void    QEImage::setVertSlice1Legend     ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_V1_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_VSLICE1, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_VSLICE1, legend ); }
QString QEImage::getVertSlice2Legend()                     { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_V2_SLICE );        }
void    QEImage::setVertSlice2Legend     ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_V2_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_VSLICE2, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_VSLICE2, legend ); }
QString QEImage::getVertSlice3Legend()                     { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_V3_SLICE );        }
void    QEImage::setVertSlice3Legend     ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_V3_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_VSLICE3, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_VSLICE3, legend ); }
QString QEImage::getVertSlice4Legend()                     { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_V4_SLICE );        }
void    QEImage::setVertSlice4Legend     ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_V4_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_VSLICE4, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_VSLICE4, legend ); }
QString QEImage::getVertSlice5Legend()                     { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_V5_SLICE );        }
void    QEImage::setVertSlice5Legend     ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_V5_SLICE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_VSLICE5, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_VSLICE5, legend ); }
QString QEImage::getprofileLegend()                       { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_LINE );           }
void    QEImage::setProfileLegend       ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_LINE,    legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_PROFILE, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_PROFILE, legend ); }
QString QEImage::getAreaSelection1Legend()                { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_REGION1 );        }
void    QEImage::setAreaSelection1Legend( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_REGION1, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_AREA1, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_AREA1, legend ); }
QString QEImage::getAreaSelection2Legend()                { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_REGION2 );        }
void    QEImage::setAreaSelection2Legend( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_REGION2, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_AREA2, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_AREA2, legend ); }
QString QEImage::getAreaSelection3Legend()                { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_REGION3 );        }
void    QEImage::setAreaSelection3Legend( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_REGION3, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_AREA3, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_AREA3, legend ); }
QString QEImage::getAreaSelection4Legend()                { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_REGION4 );        }
void    QEImage::setAreaSelection4Legend( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_REGION4, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_AREA4, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_AREA4, legend ); }
QString QEImage::getTargetLegend()                        { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_TARGET );         }
void    QEImage::setTargetLegend        ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_TARGET,  legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_TARGET, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_TARGET, legend ); }
QString QEImage::getBeamLegend()                          { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_BEAM );           }
void    QEImage::setBeamLegend          ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_BEAM,    legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_BEAM, legend );
                                                                   sMenu->setItemText( imageContextMenu::ICM_SELECT_BEAM, legend ); }
QString QEImage::getEllipseLegend()                       { return videoWidget->getMarkupLegend( imageMarkup::MARKUP_ID_ELLIPSE );       }
void    QEImage::setEllipseLegend       ( QString legend ){        videoWidget->setMarkupLegend( imageMarkup::MARKUP_ID_ELLIPSE, legend );
                                                                   mdMenu->setItemText( imageContextMenu::ICM_DISPLAY_ELLIPSE, legend );
                                                                   /* No ellipse selection mode sMenu->setItemText( imageContextMenu::ICM_SELECT_ELLIPSE, legend );*/ }

// MPEG image source URL
void QEImage::setSubstitutedUrl( QString urlIn )
{
    url = urlIn;
    mpegSource->setURL( substituteThis( url ));
}

QString QEImage::getSubstitutedUrl()
{
    return url;
}



// Full Screen property set/get
bool QEImage::getFullScreen()
{
    return fullScreen;
}

void QEImage::setFullScreen( bool fullScreenIn )
{

    // Determine the screen or screens to go fullscreen on if required
    QRect geom;
    if( fullScreenIn )
    {
        if( !screenSelectDialog::getFullscreenGeometry( this, geom ) )
        {
            return;
        }
    }

    // Save the current full screen state
    fullScreen = fullScreenIn;

    // Enter full screen
    if( fullScreen )
    {
        // Only do anything if not in designer, and no full screen window has been created already
        if( !inDesigner() && !fullScreenMainWindow )
        {
            // Create full screen window
            // (and set up context sensitive menu (right click menu))
            fullScreenMainWindow = new fullScreenWindow( this );
            fullScreenMainWindow->setContextMenuPolicy( Qt::CustomContextMenu );
            connect( fullScreenMainWindow, SIGNAL( customContextMenuRequested( const QPoint& )), this, SLOT( showImageContextMenuFullScreen( const QPoint& )));

            // Move the video widget into the full screen window
            QWidget* w = scrollArea->takeWidget();
            fullScreenMainWindow->setCentralWidget( w );

            // Select the correct screen or screens
            fullScreenMainWindow->setGeometry( geom );

            // Present the video widget in full screen
            fullScreenMainWindow->showFullScreen();

            // Raise in front of whatever application the QEImage widget is in, and resize it
            // This is only required when the QEWidget is created before being loaded into
            // some other application widget hierarchy.
            // For example, when QEGui opens a .ui file containing a QEImage widget:
            //    - The QEImage is created when the .ui file is loaded (and on creation creates and uses the full screen widget here)
            //    - QEGui inserts the widgets created from the .ui file and presents it's main window (over the top of the QEImage's full screen window)
            // Note, a timer event is not used to to wait for any particular elapsed time,
            //       but to ensure raising the full screen window occurs after an application creating
            //       this QEImage widget has finished doing whatever it is doing (which may include
            //       showing itself over the top of the full screen window.
            QTimer::singleShot( 0, this, SLOT(raiseFullScreen() ) );
        }
    }

    // Leave full screen
    else
    {
        // Only do anything if already presenting in full screen
        if( fullScreenMainWindow )
        {
            // Move the video widget back into the scroll area within the QEImage
            QWidget* w = fullScreenMainWindow->centralWidget();
            scrollArea->setWidget( w );

            // Destroy the fullscreen main window
            delete fullScreenMainWindow;
            fullScreenMainWindow = NULL;
        }
    }
}

// Ensure the full screen main window is in front of the application that created the QEImage widget,
// and resized to fit the screen.
// This is called as a timer event, not to create a delay (time is zero) but to ensure it is called after back in event loop
void QEImage::raiseFullScreen()
{
    if( fullScreenMainWindow )
    {
        fullScreenMainWindow->activateWindow();
        fullScreenMainWindow->raise();
        fullScreenMainWindow->setFocus();

        // Resize to fit (or current image view will be stretched over the screen size, which may affect aspect ratio
        // Note, done as a timer event of 100mS, not to wait any particular time,
        // but to ensure all events related to window activation, raising, and receiving focus generated
        // within this timer event have occured first.
        // There should be a more deterministic way to ensure this!!!
        QTimer::singleShot( 100, this, SLOT(resizeFullScreen() ) );
    }
}

// Resize full screen once it has been managed
void QEImage::resizeFullScreen()
{
    setResizeOption( RESIZE_OPTION_FIT );
}

//=================================================================================================

void QEImage::panModeClicked()
{
    videoWidget->setMode(  imageMarkup::MARKUP_ID_NONE );
    videoWidget->setPanning( true );
}

void QEImage::vSlice1SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_V1_SLICE );
}

void QEImage::vSlice2SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_V2_SLICE );
}

void QEImage::vSlice3SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_V3_SLICE );
}

void QEImage::vSlice4SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_V4_SLICE );
}

void QEImage::vSlice5SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_V5_SLICE );
}

void QEImage::hSlice1SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_H1_SLICE );
}

void QEImage::hSlice2SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_H2_SLICE );
}

void QEImage::hSlice3SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_H3_SLICE );
}

void QEImage::hSlice4SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_H4_SLICE );
}

void QEImage::hSlice5SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_H5_SLICE );
}

void QEImage::area1SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_REGION1 );
}

void QEImage::area2SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_REGION2 );
}

void QEImage::area3SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_REGION3 );
}

void QEImage::area4SelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_REGION4 );
}

void QEImage::profileSelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_LINE );
}

void QEImage::targetSelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_TARGET );
}

void QEImage::beamSelectModeClicked()
{
    videoWidget->setPanning( false );
    videoWidget->setMode(  imageMarkup::MARKUP_ID_BEAM);
}
//=================================================================================================

void QEImage::zoomInOut( int zoomAmount )
{
    setResizeOption( RESIZE_OPTION_ZOOM );
    double oldZoom = zoom;
    double newZoom = zoom + zoomAmount;
    setZoom( int( newZoom ) );

    double currentScrollPosX = scrollArea->horizontalScrollBar()->value();
    double currentScrollPosY = scrollArea->verticalScrollBar()->value();
    double newScrollPosX = currentScrollPosX *newZoom / oldZoom;
    double newScrollPosY = currentScrollPosY *newZoom / oldZoom;

    scrollArea->horizontalScrollBar()->setValue( int( newScrollPosX ) );
    scrollArea->verticalScrollBar()->setValue( int( newScrollPosY ) );
}

// The user has made (or is making) a selection in the displayed image.
// Act on the selelection
void QEImage::userSelection( imageMarkup::markupIds mode,   // Markup being manipulated
                             bool complete,                 // True if the user has completed an operation (for example, finished moving a markup to a new position and a write to a variable is now required)
                             bool clearing,                 // True if a markup is being cleared
                             QPoint point1,                 // Generic first point of the markup. for example, to left of an area, or target position
                             QPoint point2,                 // Optional generic second point of the markup
                             unsigned int thickness )       // Optional thickness of the markup
{
    // If creating or moving a markup...
    if( !clearing )
    {
        switch( mode )
        {
            case imageMarkup::MARKUP_ID_V1_SLICE:
                vSlice1X = point1.x();
                vSlice1Thickness = thickness;
                haveVSlice1X = true;

                // Only first vertical slice has profile data
                if( enableVertSlicePresentation )
                {
                    QTimer::singleShot( 0, this, SLOT(setVSliceControlsVisible() ) );
                    generateVSlice(  vSlice1X, vSlice1Thickness );
                }

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE1, true );

                vertProfile1Changed();
                break;

            case imageMarkup::MARKUP_ID_V2_SLICE:
                vSlice2X = point1.x();
                vSlice2Thickness = thickness;
                haveVSlice2X = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE2, true );

                vertProfile2Changed();
                break;

            case imageMarkup::MARKUP_ID_V3_SLICE:
                vSlice3X = point1.x();
                vSlice3Thickness = thickness;
                haveVSlice3X = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE3, true );

                vertProfile3Changed();
                break;

            case imageMarkup::MARKUP_ID_V4_SLICE:
                vSlice4X = point1.x();
                vSlice4Thickness = thickness;
                haveVSlice4X = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE4, true );

                vertProfile4Changed();
                break;

            case imageMarkup::MARKUP_ID_V5_SLICE:
                vSlice5X = point1.x();
                vSlice5Thickness = thickness;
                haveVSlice5X = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE5, true );

                vertProfile5Changed();
                break;

            case imageMarkup::MARKUP_ID_H1_SLICE:
                hSlice1Y = point1.y();
                hSlice1Thickness = thickness;
                haveHSlice1Y = true;

                // Only first horizontal slice has profile data
                if( enableHozSlicePresentation )
                {
                    QTimer::singleShot( 0, this, SLOT(setHSliceControlsVisible() ) );
                    generateHSlice( hSlice1Y, hSlice1Thickness );
                }

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE1, true );

                hozProfile1Changed();
                break;

            case imageMarkup::MARKUP_ID_H2_SLICE:
                hSlice2Y = point1.y();
                hSlice2Thickness = thickness;
                haveHSlice2Y = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE2, true );

                hozProfile2Changed();
                break;

            case imageMarkup::MARKUP_ID_H3_SLICE:
                hSlice3Y = point1.y();
                hSlice3Thickness = thickness;
                haveHSlice3Y = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE3, true );

                hozProfile3Changed();
                break;

            case imageMarkup::MARKUP_ID_H4_SLICE:
                hSlice4Y = point1.y();
                hSlice4Thickness = thickness;
                haveHSlice4Y = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE4, true );

                hozProfile4Changed();
                break;

            case imageMarkup::MARKUP_ID_H5_SLICE:
                hSlice5Y = point1.y();
                hSlice5Thickness = thickness;
                haveHSlice5Y = true;

                // Is this OK outside if????!!!!!
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE5, true );

                hozProfile5Changed();
                break;

            case imageMarkup::MARKUP_ID_REGION1:
                selectedArea1Point1 = point1;
                selectedArea1Point2 = point2;
                haveSelectedArea1 = true;

                zMenu->enableAreaSelected( haveSelectedArea1 );

                displaySelectedAreaInfo( 1, point1, point2 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA1, true );
                if( imageDisplayProps && imageDisplayProps->getAutoBrightnessContrast() )
                {
                    setRegionAutoBrightnessContrast( point1, point2 );
                }

                if( complete )
                {
                    roi1Changed();
                }
                break;

            case imageMarkup::MARKUP_ID_REGION2:
                selectedArea2Point1 = point1;
                selectedArea2Point2 = point2;
                haveSelectedArea2 = true;

                displaySelectedAreaInfo( 2, point1, point2 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA2, true );
                if( imageDisplayProps && imageDisplayProps->getAutoBrightnessContrast() )
                {
                    setRegionAutoBrightnessContrast( point1, point2 );
                }

                if( complete )
                {
                    roi2Changed();
                }
                break;

            case imageMarkup::MARKUP_ID_REGION3:
                selectedArea3Point1 = point1;
                selectedArea3Point2 = point2;
                haveSelectedArea3 = true;

                displaySelectedAreaInfo( 3, point1, point2 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA3, true );
                if( imageDisplayProps && imageDisplayProps->getAutoBrightnessContrast() )
                {
                    setRegionAutoBrightnessContrast( point1, point2 );
                }

                if( complete )
                {
                    roi3Changed();
                }
                break;

            case imageMarkup::MARKUP_ID_REGION4:
                selectedArea4Point1 = point1;
                selectedArea4Point2 = point2;
                haveSelectedArea4 = true;

                displaySelectedAreaInfo( 4, point1, point2 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA4, true );
                if( imageDisplayProps && imageDisplayProps->getAutoBrightnessContrast() )
                {
                    setRegionAutoBrightnessContrast( point1, point2 );
                }

                if( complete )
                {
                    roi4Changed();
                }
                break;

            case imageMarkup::MARKUP_ID_LINE:
                profileLineStart = point1;
                profileLineEnd = point2;
                profileThickness = thickness;
                haveProfileLine = true;
                if( enableProfilePresentation )
                {
                    QTimer::singleShot( 0, this, SLOT(setLineProfileControlsVisible() ) );
                    generateProfile( profileLineStart, profileLineEnd, profileThickness );
                    mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_PROFILE, true );
                }

                lineProfileChanged();
                break;

            case imageMarkup::MARKUP_ID_TARGET:
                {
                    targetInfo.setPoint( iProcessor.rotateFlipToDataPoint( point1 ) );

                    // Write the target variables.
                    QEInteger *qca;
                    qca = (QEInteger*)getQcaItem( TARGET_X_VARIABLE );
                    if( qca ) qca->writeInteger( targetInfo.getPoint().x() );

                    qca = (QEInteger*)getQcaItem( TARGET_Y_VARIABLE );
                    if( qca ) qca->writeInteger( targetInfo.getPoint().y() );

                    // Display textual info
                    infoUpdateTarget( targetInfo.getPoint().x(), targetInfo.getPoint().y() );

                    // Update markup display menu
                    mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_TARGET, true );
                }
                break;

            case imageMarkup::MARKUP_ID_BEAM:
                {
                    beamInfo.setPoint( iProcessor.rotateFlipToDataPoint( point1 ) );

                    // Write the beam variables.
                    QEInteger *qca;
                    qca = (QEInteger*)getQcaItem( BEAM_X_VARIABLE );
                    if( qca ) qca->writeInteger( beamInfo.getPoint().x() );

                    qca = (QEInteger*)getQcaItem( BEAM_Y_VARIABLE );
                    if( qca ) qca->writeInteger( beamInfo.getPoint().y() );

                    // Display textual info
                    infoUpdateBeam( beamInfo.getPoint().x(), beamInfo.getPoint().y() );

                    // Update markup display menu
                    mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_BEAM, true );
                }
                break;

            default:
                break;

        }
    }

    // If clearing a markup...
    else
    {
        switch( mode )
        {
            case imageMarkup::MARKUP_ID_V1_SLICE:
                vSlice1X = 0;
                haveVSlice1X = false;

                QTimer::singleShot( 0, this, SLOT(setVSliceControlsNotVisible() ) );  // Only for first slice
                infoUpdateVertProfile();    // Only for first slice

                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE1, false );
                break;

            case imageMarkup::MARKUP_ID_V2_SLICE:
                vSlice2X = 0;
                haveVSlice2X = false;

                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE2, false );
                break;

            case imageMarkup::MARKUP_ID_V3_SLICE:
                vSlice3X = 0;
                haveVSlice3X = false;

                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE3, false );
                break;

            case imageMarkup::MARKUP_ID_V4_SLICE:
                vSlice4X = 0;
                haveVSlice4X = false;

                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE4, false );
                break;

            case imageMarkup::MARKUP_ID_V5_SLICE:
                vSlice5X = 0;
                haveVSlice5X = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_VSLICE5, false );
                break;

            case imageMarkup::MARKUP_ID_H1_SLICE:
                hSlice1Y = 0;
                haveHSlice1Y = false;

                QTimer::singleShot( 0, this, SLOT(setHSliceControlsNotVisible() ) );  // Only for first slice
                infoUpdateHozProfile();  // Only for first slice

                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE1, false );
                break;

            case imageMarkup::MARKUP_ID_H2_SLICE:
                hSlice2Y = 0;
                haveHSlice2Y = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE2, false );
                break;

            case imageMarkup::MARKUP_ID_H3_SLICE:
                hSlice3Y = 0;
                haveHSlice3Y = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE3, false );
                break;

            case imageMarkup::MARKUP_ID_H4_SLICE:
                hSlice4Y = 0;
                haveHSlice4Y = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE4, false );
                break;

            case imageMarkup::MARKUP_ID_H5_SLICE:
                hSlice5Y = 0;
                haveHSlice5Y = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_HSLICE5, false );
                break;

            case imageMarkup::MARKUP_ID_REGION1:
                selectedArea1Point1 = QPoint();
                selectedArea1Point2 = QPoint();
                haveSelectedArea1 = false;
                infoUpdateRegion( 1 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA1, false );

                zMenu->enableAreaSelected( haveSelectedArea1 );
                break;

            case imageMarkup::MARKUP_ID_REGION2:
                selectedArea2Point1 = QPoint();
                selectedArea2Point2 = QPoint();
                haveSelectedArea2 = false;
                infoUpdateRegion( 2 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA2, false );
                break;

            case imageMarkup::MARKUP_ID_REGION3:
                selectedArea3Point1 = QPoint();
                selectedArea3Point2 = QPoint();
                haveSelectedArea3 = false;
                infoUpdateRegion( 3 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA3, false );
                break;

            case imageMarkup::MARKUP_ID_REGION4:
                selectedArea4Point1 = QPoint();
                selectedArea4Point2 = QPoint();
                haveSelectedArea4 = false;
                infoUpdateRegion( 4 );
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_AREA4, false );
                break;

            case imageMarkup::MARKUP_ID_LINE:
                profileLineStart = QPoint();
                profileLineEnd = QPoint();
                haveProfileLine = false;
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_PROFILE, false );

                QTimer::singleShot( 0, this, SLOT(setLineProfileControlsNotVisible() ) );
                infoUpdateProfile();
                break;

            case imageMarkup::MARKUP_ID_TARGET:
                infoUpdateTarget();
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_TARGET, false );
                break;

            case imageMarkup::MARKUP_ID_BEAM:
                infoUpdateBeam();
                mdMenu->setDisplayed( imageContextMenu::ICM_DISPLAY_BEAM, false );
                break;

            default:
                break;

        }
    }
}

// Redisplay all markups
// Used when rotating / flipping image
void QEImage::redisplayAllMarkups()
{
    // Apply all markup data to the new rotation/flip
    // (This will re-display any markups with data making some of the the redrawing of all markups below redundant)
    useAllMarkupData();

    // Redraw all markups
    videoWidget->markupChange();

}

//==================================================
// Slots to make profile plots appear or disappear
// They are used as timer events to ensure resize events (that happen as the controls are inserted or deleted)
// don't cause a redraw of markups while handling a markup draw event

// Slot to make vertical slice profile plot appear
void QEImage::setVSliceControlsVisible()
{
    // Do nothing if not presenting this control.
    // Even though this method is not called unless presentation is required, it may be called as a
    // timer event to ensure it is called after all events are processed, which may mean properties have changed
    if( !enableVertSlicePresentation )
    {
        return;
    }

    // Make vertical slice profile plot appear
    if( !appHostsControls )
    {
        vSliceLabel->setVisible( true );
    }
    if( vSliceDisplay )
    {
        vSliceDisplay->setVisible( true );
    }
}

// Slot to make vertical slice profile plot disapear
void QEImage::setVSliceControlsNotVisible()
{
    vSliceLabel->setVisible( false );
    if( vSliceDisplay )
    {
        vSliceDisplay->setVisible( false );
    }
}

// Slot to make horizontal slice profile plot appear
void QEImage::setHSliceControlsVisible()
{
    // Do nothing if not presenting this control.
    // Even though this method is not called unless presentation is required, it may be called as a
    // timer event to ensure it is called after all events are processed, which may mean properties have changed
    if( !enableHozSlicePresentation )
    {
        return;
    }

    // Make horizontal slice profile plot appear
    if( !appHostsControls )
    {
        hSliceLabel->setVisible( true );
    }
    if( hSliceDisplay )
    {
        hSliceDisplay->setVisible( true );
    }
}

// Slot to make horizontal profile plot disapear
void QEImage::setHSliceControlsNotVisible()
{
    hSliceLabel->setVisible( false );
    if( hSliceDisplay )
    {
        hSliceDisplay->setVisible( false );
    }
}

// Slot to make arbitrary line profile plot appear
void QEImage::setLineProfileControlsVisible()
{
    // Do nothing if not presenting this control.
    // Even though this method is not called unless presentation is required, it may be called as a
    // timer event to ensure it is called after all events are processed, which may mean properties have changed
    if( !enableProfilePresentation )
    {
        return;
    }

    // Make arbitrary line profile plot appear
    if( !appHostsControls )
    {
        profileLabel->setVisible( true );
    }
    if( profileDisplay )
    {
        profileDisplay->setVisible( true );
    }
}

// Slot to make arbitrary line profile plot disapear
void QEImage::setLineProfileControlsNotVisible()
{
    profileLabel->setVisible( false );
    if( profileDisplay )
    {
        profileDisplay->setVisible( false );
    }
}

//==================================================

// Display textual info about a selected area
//!!! No longer needed. change calls to displaySelectedAreaInfo() to calls to infoUpdateRegion() directly
void QEImage::displaySelectedAreaInfo( int region, QPoint point1, QPoint point2 )
{
    infoUpdateRegion( region, point1.x(), point1.y(), point2.x(), point2.y() );
}

// Update the brightness and contrast, if in auto, to match the recently selected region
void QEImage::setRegionAutoBrightnessContrast( QPoint point1, QPoint point2 )
{
    // Translate the corners to match the current flip and roate options
    QRect area = iProcessor.rotateFlipToDataRectangle( point1, point2 );

    // Determine the range of pixel values in the selected area
    unsigned int min, max;
    iProcessor.getPixelRange( area, &min, &max );

    if( imageDisplayProps )
    {
        imageDisplayProps->setBrightnessContrast( max, min );
    }
}

//=====================================================================
// Slots to use signals from the Brightness/contrast control

// The brightness or contrast or contrast reversal has changed
void QEImage::imageDisplayPropertiesChanged()
{
    // Flag that the current pixel lookup table needs recalculating
    iProcessor.invalidatePixelLookup();

    // Present the updated image
    displayImage();
}

// A request has been made to set the brightness and contrast to suit the current image
void QEImage::brightnessContrastAutoImageRequest()
{
    setRegionAutoBrightnessContrast( QPoint( 0, 0), QPoint( iProcessor.getImageBuffWidth(), iProcessor.getImageBuffHeight() ) );
}

//=====================================================================

// Generate a profile along a line down an image at a given X position
// Input ordinates are scaled to the source image data.
// The profile contains values for each pixel intersected by the line.
void QEImage::generateVSlice( int x, unsigned int thickness )
{
    if( !vSliceDisplay )
    {
        return;
    }

    // Display textual info
    infoUpdateVertProfile( x, thickness );

    // If not over the image, remove the profile
    if( x < 0 || x >= (int)iProcessor.rotatedImageBuffWidth() )
    {
        vSliceDisplay->clearProfile();
        return;
    }

    // Generate the data through the slice
    iProcessor.generateVSliceData( vSliceData, x, thickness );

    // Write the profile data
    QEFloating *qca;
    qca = (QEFloating*)getQcaItem( PROFILE_V_ARRAY );
    if( qca )
    {
        int arraySize = vSliceData.size();
        QVector<double> waveform(arraySize);
        for( int i = 0; i < arraySize; i++ )
        {
            waveform[i] = vSliceData[i].x();
        }
        qca->writeFloating( waveform );
    }

    // Display the profile
    QDateTime dt = QDateTime::currentDateTime();
    QString title = QString( "Vertical profile - " )
                    .append( getSubstitutedVariableName( IMAGE_VARIABLE ) )
                    .append( dt.toString(" - dd.MM.yyyy HH:mm:ss.zzz") );
    vSliceDisplay->setProfile( &vSliceData,
                               iProcessor.maxPixelValue(),
                               0.0,
                               (double)(vSliceData.size()),
                               0.0,
                               title,
                               QPoint( x, 0 ),
                               QPoint( x, iProcessor.rotatedImageBuffHeight()-1 ),
                               thickness );
}

// Generate a profile along a line across an image at a given Y position
// Input ordinates are at the resolution of the source image data
// The profile contains values for each pixel intersected by the line.
void QEImage::generateHSlice( int y, unsigned int thickness )
{
    if( !hSliceDisplay )
    {
        return;
    }

    // Display textual info
    infoUpdateHozProfile( y, thickness );

    // If not over the image, remove the profile
    if( y < 0 || y >= (int)iProcessor.rotatedImageBuffHeight() )
    {
        hSliceDisplay->clearProfile();
        return;
    }

    // Generate the data through the slice
    iProcessor.generateHSliceData( hSliceData, y, thickness );

    // Write the profile data
    QEFloating *qca;
    qca = (QEFloating*)getQcaItem( PROFILE_H_ARRAY );
    if( qca )
    {
        int arraySize = hSliceData.size();
        QVector<double> waveform(arraySize);
        for( int i = 0; i < arraySize; i++ )
        {
            waveform[i] = hSliceData[i].y();
        }
        qca->writeFloating( waveform );
    }

    // Display the profile
    QDateTime dt = QDateTime::currentDateTime();
    QString title = QString( "Horizontal profile - " )
                    .append( getSubstitutedVariableName( IMAGE_VARIABLE ) )
                    .append( dt.toString(" - dd.MM.yyyy HH:mm:ss.zzz") );
    hSliceDisplay->setProfile( &hSliceData,
                               0.0,
                               (double)(hSliceData.size()),
                               0.0,
                               iProcessor.maxPixelValue(),
                               title,
                               QPoint( y, 0 ),
                               QPoint( y, iProcessor.rotatedImageBuffWidth()-1 ),
                               thickness );
}

// Generate a profile along an arbitrary line through an image.
// Input ordinates are scaled to the source image data.
// The profile contains values one pixel length along the line.
// Except where the line is vertical or horizontal points one pixel
// length along the line will not line up with actual pixels.
// The values returned are a weighted average of the four actual pixels
// containing a notional pixel drawn around the each point on the line.
//
// In the example below, a line was drawn from pixels (1,1) to (3,3).
//
// The starting and ending points are the center of the start and end
// pixels: (1.5,1.5)  (3.5,3.5)
//
// The points along the line one pixel length apart are roughly at points
// (1.5,1.5) (2.2,2.2) (2.9,2.9) (3.6,3.6)
//
// The points are marked in the example with an 'x'.
//
//     0       1       2       3       4
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 0 |       |       |       |       |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 1 |       |   x ......... |       |       |
//   |       |     . |     . |       |       |
//   +-------+-----.-+-----.-+-------+-------+
//   |       |     . | x   . |       |       |
// 2 |       |     . |     . |       |       |
//   |       |     .........x|       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 3 |       |       |       |   x   |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 4 |       |       |       |       |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//
// The second point has a notional pixel drawn around it like so:
//      .........
//      .       .
//      .       .
//      .   x   .
//      .       .
//      .........
//
// This notional pixel overlaps pixels (1,1) (1,2) (2,1) and (2,2).
//
// The notional pixel overlaps about 10% of pixel (1,1),
// 20% of pixels (1,2) and (2,1) and 50% of pixel (2,2).
//
// A value for the second point will be the sum of the four pixels
// overlayed by the notional pixel weighted by these values.
//
// The line has a notional thickness. The above processing for a single
// pixel width is repeated with the start and end points moved at right
// angles to the line by a 'pixel' distance up to the line thickness.
// The results are then averaged.
//
void QEImage::generateProfile( QPoint point1, QPoint point2, unsigned int thickness )
{
    if( !profileDisplay )
    {
        return;
    }

    // Display textual information
    infoUpdateProfile( point1, point2, thickness );

    // X and Y components of line drawn
    double dX = point2.x()-point1.x();
    double dY = point2.y()-point1.y();

    // Do nothing if no line
    if( dX == 0 && dY == 0 )
    {
        profileDisplay->clearProfile();
        return;
    }

    // Generate the data through the slice
    iProcessor.generateProfileData( profileData, point1, point2, thickness );

    // Write the profile data
    QEFloating *qca;
    qca = (QEFloating*)getQcaItem( PROFILE_LINE_ARRAY );
    if( qca )
    {
        int arraySize = profileData.size();
        QVector<double> waveform(arraySize);
        for( int i = 0; i < arraySize; i++ )
        {
            waveform[i] = profileData[i].y();
        }
        qca->writeFloating( waveform );
    }

    // Update the profile display
    QDateTime dt = QDateTime::currentDateTime();
    QString title = QString( "Line profile - " ).append( getSubstitutedVariableName( IMAGE_VARIABLE ) ).append( dt.toString(" - dd.MM.yyyy HH:mm:ss.zzz") );
    profileDisplay->setProfile( &profileData, 0.0, (double)(profileData.size()), 0.0,  iProcessor.maxPixelValue(), title, point1, point2, thickness );
}

//=================================================================================================
// Display a pixel value.
void QEImage::currentPixelInfo( QPoint pos )
{
    // Don't do anything if no image data yet
    if( !iProcessor.hasImage() )
    {
        return;
    }

    // If the pixel is not within the image, display nothing
    QString s;
    if( pos.x() < 0 || pos.y() < 0 || pos.x() >= (int)iProcessor.rotatedImageBuffWidth() || pos.y() >= (int)iProcessor.rotatedImageBuffHeight() )
    {
        infoUpdatePixel();
    }

    // If the pixel is within the image, display the pixel position and value
    else
    {
        // Extract the pixel data from the original image data
        int value = iProcessor.getPixelValueFromData( iProcessor.getImageDataPtr( pos ) );
        infoUpdatePixel( pos, value );
    }
}

// Pan the image.
// This is used when:
//   - Zooming to a selected area (zoom to the right level, then call this
//     method to move the selected area into view).
//   - Resetting the scroll bars after the user has panned by dragging the image.
//
// Note: when the user is panning by dragging the image, this method is only used to tidy
// up the scroll bars at the end of the pan.
// Panning has been done by moving the VideoWidget in the viewport directly (not via the
// scroll bars) as the VideoWidget can be moved directly more smoothly to pixel resolution,
// whereas the VideoWidget can only be moved by the resolution of a scrollbar step when moved
// by setting the scroll bar values.
// A consequence of this is, however, the scroll bars are left where ever they were
// when panning started. This function will set the scroll bars to match the new
// VideoWidget position. Note, if the scroll bar values are changed here, this will itself
// cause the VideoWidget to pan, but only from the pixel accurate position set by the
// direct scan to a close pixel determined by the scroll bar pixel resolution.
// Note, the VideoWidget can be panned with the mouse beyond the scroll range. If either
// scroll bar value is changed here the VideoWidget will be pulled back within the scroll
// bar range. If neither scroll bar value changes here, the VideoWidget is left panned
// beyond the scroll bar range. To demonstrate this, set both scroll bars to zero,
// then pan the viewport down and to the right with the mouse.
void QEImage::pan( QPoint origin )
{
    // Determine the proportion of the scroll bar maximums to set the scroll bar to.
    // The scroll bar will be zero when the VideoWidget origin is zero, and maximum when the
    // part of the VideoWidget past the origin equals the viewport size.
    QSize vpSize = scrollArea->viewport()->size();

    double xProportion = (double)-origin.x()/(double)(videoWidget->width()-vpSize.width());
    double yProportion = (double)-origin.y()/(double)(videoWidget->height()-vpSize.height());

    xProportion = (xProportion<0.0)?0.0:xProportion;
    yProportion = (yProportion<0.0)?0.0:yProportion;

    xProportion = (xProportion>1.0)?1.0:xProportion;
    yProportion = (yProportion>1.0)?1.0:yProportion;

    // Update the scroll bars to match the panning
    scrollArea->horizontalScrollBar()->setValue( int( scrollArea->horizontalScrollBar()->maximum() * xProportion ) );
    scrollArea->verticalScrollBar()->setValue( int( scrollArea->verticalScrollBar()->maximum() * yProportion ) );
}

//=================================================================================================
// Slot to redraw the current image.
// Required when properties change, such as contrast reversal, or when the video widget changes, such as a resize
void QEImage::redraw()
{
    qcaobject::QCaObject* qca = getQcaItem( IMAGE_VARIABLE );
    if( qca )
    {
        qca->resendLastData();
    }
}

//=================================================================================================
// Present the context menu
// (When in full screen)
void QEImage::showImageContextMenuFullScreen( const QPoint& pos )
{
    QPoint globalPos = fullScreenMainWindow->mapToGlobal( pos );
    showImageContextMenuCommon( pos, globalPos );
}

// Present the context menu
// (When not in full screen)
void QEImage::showImageContextMenu( const QPoint& pos )
{
    QPoint globalPos = mapToGlobal( pos );
    showImageContextMenuCommon( pos, globalPos );
}

// Present the context menu
// (full screen and not full screen)
void QEImage::showImageContextMenuCommon( const QPoint& pos, const QPoint& globalPos )
{
    // If the markup system wants to put up a menu, let it do so
    // For example, if the user has clicked over a markup, it may offer the user a menu
    if( videoWidget->showMarkupMenu( videoWidget->mapFrom( this, pos ), globalPos ) )
    {
        return;
    }

    if( fullContextMenu )
    {
        // Create the standard context menu
        QMenu* cm = buildContextMenu();

        // Add the Selection menu
        cm->addSeparator();
        sMenu->setChecked( getSelectionOption() );
        cm->addMenu( sMenu );
        cm->addMenu( mdMenu );

        // Add menu items

        //                    Title                            checkable  checked                     option
        addMenuItem( cm,      "Save...",                       false,     false,                      imageContextMenu::ICM_SAVE                     );
        addMenuItem( cm,      paused?"Resume":"Pause",         true,      paused,                     imageContextMenu::ICM_PAUSE                    );

        addMenuItem( cm,      "About image...",                false,     false,                      imageContextMenu::ICM_ABOUT_IMAGE              );

        // Add the zoom menu
        zMenu->enableAreaSelected( haveSelectedArea1 );
        cm->addMenu( zMenu );

        // Add the flip/rotate menu
        frMenu->setChecked( iProcessor.getRotation(), iProcessor.getFlipHoz(), iProcessor.getFlipVert() );
        cm->addMenu( frMenu );

        // Add 'full scree' item
        addMenuItem( cm,      "Full Screen",                   true,      fullScreen,                 imageContextMenu::ICM_FULL_SCREEN              );

        // Add option... dialog
        addMenuItem( cm,      "Options...",                    false,     false,                      imageContextMenu::ICM_OPTIONS                  );

        // Present the menu
        imageContextMenu::imageContextMenuOptions option;
        bool checked;
        QAction* selectedItem = showContextMenuGlobal( cm, globalPos );
        if( selectedItem )
        {
            option = (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt());
            checked = selectedItem->isChecked();
        }
        else
        {
            option = imageContextMenu::ICM_NONE;
            checked = false;
        }

        // Act on the selected option.
        // (there won't be one if a standard context menu option was selected)
        optionAction( option, checked );
    }
    else
    {
        showContextMenuGlobal( globalPos );
    }

}

// Act on a selection from the option menu or dialog
void QEImage::optionAction( imageContextMenu::imageContextMenuOptions option, bool checked )
{
    // Act on the menu selection
    switch( option )
    {
        default:
        case imageContextMenu::ICM_NONE: break;

        case imageContextMenu::ICM_SAVE:                             saveClicked();                             break;
        case imageContextMenu::ICM_PAUSE:                            pauseClicked();                            break;
        case imageContextMenu::ICM_ENABLE_CURSOR_PIXEL:              showInfo                  ( checked );     break;
        case imageContextMenu::ICM_ABOUT_IMAGE:                      showImageAboutDialog();                    break;
        case imageContextMenu::ICM_ENABLE_TIME:                      videoWidget->setShowTime  ( checked );     break;
        case imageContextMenu::ICM_ENABLE_VERT1:                     doEnableVertSlice1Selection( checked );    break;
        case imageContextMenu::ICM_ENABLE_VERT2:                     doEnableVertSlice2Selection( checked );    break;
        case imageContextMenu::ICM_ENABLE_VERT3:                     doEnableVertSlice3Selection( checked );    break;
        case imageContextMenu::ICM_ENABLE_VERT4:                     doEnableVertSlice4Selection( checked );    break;
        case imageContextMenu::ICM_ENABLE_VERT5:                     doEnableVertSlice5Selection( checked );    break;
        case imageContextMenu::ICM_ENABLE_HOZ1:                      doEnableHozSlice1Selection ( checked );    break;
        case imageContextMenu::ICM_ENABLE_HOZ2:                      doEnableHozSlice2Selection ( checked );    break;
        case imageContextMenu::ICM_ENABLE_HOZ3:                      doEnableHozSlice3Selection ( checked );    break;
        case imageContextMenu::ICM_ENABLE_HOZ4:                      doEnableHozSlice4Selection ( checked );    break;
        case imageContextMenu::ICM_ENABLE_HOZ5:                      doEnableHozSlice5Selection ( checked );    break;
        case imageContextMenu::ICM_ENABLE_AREA1:                     doEnableAreaSelection      ( checked );    break;
        case imageContextMenu::ICM_ENABLE_AREA2:                     doEnableAreaSelection      ( checked );    break;
        case imageContextMenu::ICM_ENABLE_AREA3:                     doEnableAreaSelection      ( checked );    break;
        case imageContextMenu::ICM_ENABLE_AREA4:                     doEnableAreaSelection      ( checked );    break;
        case imageContextMenu::ICM_ENABLE_LINE:                      doEnableProfileSelection   ( checked );    break;
        case imageContextMenu::ICM_ENABLE_TARGET:                    doEnableTargetSelection    ( checked );    break;
        case imageContextMenu::ICM_ENABLE_BEAM:                      doEnableBeamSelection      ( checked );    break;
        case imageContextMenu::ICM_DISPLAY_BUTTON_BAR:               doEnableButtonBar          ( checked );    break;
        case imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES: doEnableImageDisplayProperties( checked ); break;
        case imageContextMenu::ICM_DISPLAY_RECORDER:                 doEnableRecording          ( checked );    break;
        case imageContextMenu::ICM_FULL_SCREEN:                      setFullScreen              ( checked );    break;
        case imageContextMenu::ICM_OPTIONS:                          optionsDialog->exec( this );               break;

        // Note, zoom options caught by zoom menu signal
        // Note, rotate and flip options caught by flip rotate menu signal
    }
}

// Act on a selection from the zoom menu
void QEImage::zoomMenuTriggered( QAction* selectedItem )
{
    switch( (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt()) )
    {
        default:
        case imageContextMenu::ICM_NONE: break;

        case imageContextMenu::ICM_ZOOM_SELECTED:       zoomToArea();                           break;
        case imageContextMenu::ICM_ZOOM_FIT:            setResizeOption( RESIZE_OPTION_FIT );   break;
        case imageContextMenu::ICM_ZOOM_PLUS:           zoomInOut( 10 );                        break;
        case imageContextMenu::ICM_ZOOM_MINUS:          zoomInOut( -10 );                       break;
        case imageContextMenu::ICM_ZOOM_10:             setResizeOptionAndZoom(  10 );          break;
        case imageContextMenu::ICM_ZOOM_25:             setResizeOptionAndZoom(  25 );          break;
        case imageContextMenu::ICM_ZOOM_50:             setResizeOptionAndZoom(  50 );          break;
        case imageContextMenu::ICM_ZOOM_75:             setResizeOptionAndZoom(  75 );          break;
        case imageContextMenu::ICM_ZOOM_100:            setResizeOptionAndZoom( 100 );          break;
        case imageContextMenu::ICM_ZOOM_150:            setResizeOptionAndZoom( 150 );          break;
        case imageContextMenu::ICM_ZOOM_200:            setResizeOptionAndZoom( 200 );          break;
        case imageContextMenu::ICM_ZOOM_300:            setResizeOptionAndZoom( 300 );          break;
        case imageContextMenu::ICM_ZOOM_400:            setResizeOptionAndZoom( 400 );          break;
    }
}

// Act on a selection from the flip/rotate menu
void QEImage::flipRotateMenuTriggered( QAction* selectedItem )
{
    switch( (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt()) )
    {
        default:
        case imageContextMenu::ICM_NONE: break;

        case imageContextMenu::ICM_ROTATE_RIGHT:        setRotation( selectedItem->isChecked()?imageProperties::ROTATION_90_RIGHT:imageProperties::ROTATION_0 ); break;
        case imageContextMenu::ICM_ROTATE_LEFT:         setRotation( selectedItem->isChecked()?imageProperties::ROTATION_90_LEFT :imageProperties::ROTATION_0 ); break;
        case imageContextMenu::ICM_ROTATE_180:          setRotation( selectedItem->isChecked()?imageProperties::ROTATION_180     :imageProperties::ROTATION_0 ); break;

        case imageContextMenu::ICM_FLIP_HORIZONTAL:     setHorizontalFlip( selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_FLIP_VERTICAL:       setVerticalFlip  ( selectedItem->isChecked() ); break;
    }

    // Update the checked state of the buttons now the user has selected an option.
    // Note, this is also called before displaying the menu to reflect any property
    // changes from other sources
    frMenu->setChecked( iProcessor.getRotation(), iProcessor.getFlipHoz(), iProcessor.getFlipVert() );
}

// Act on a selection from the select menu
void QEImage::selectMenuTriggered( QAction* selectedItem )
{
    switch( (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt()) )
    {
        default:
        case imageContextMenu::ICM_NONE: break;

        case imageContextMenu::ICM_SELECT_PAN:          panModeClicked();           break;
        case imageContextMenu::ICM_SELECT_VSLICE1:      vSlice1SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_VSLICE2:      vSlice2SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_VSLICE3:      vSlice3SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_VSLICE4:      vSlice4SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_VSLICE5:      vSlice5SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_HSLICE1:      hSlice1SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_HSLICE2:      hSlice2SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_HSLICE3:      hSlice3SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_HSLICE4:      hSlice4SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_HSLICE5:      hSlice5SelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_AREA1:        area1SelectModeClicked();   break;
        case imageContextMenu::ICM_SELECT_AREA2:        area2SelectModeClicked();   break;
        case imageContextMenu::ICM_SELECT_AREA3:        area3SelectModeClicked();   break;
        case imageContextMenu::ICM_SELECT_AREA4:        area4SelectModeClicked();   break;
        case imageContextMenu::ICM_SELECT_PROFILE:      profileSelectModeClicked(); break;
        case imageContextMenu::ICM_SELECT_TARGET:       targetSelectModeClicked();  break;
        case imageContextMenu::ICM_SELECT_BEAM:         beamSelectModeClicked();    break;
    }
}

// Act on a selection from the markup display menu
void QEImage::markupDisplayMenuTriggered( QAction* selectedItem )
{
    switch( (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt()) )
    {
        default:
        case imageContextMenu::ICM_NONE: break;

        case imageContextMenu::ICM_DISPLAY_VSLICE1:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V1_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_VSLICE2:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V2_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_VSLICE3:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V3_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_VSLICE4:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V4_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_VSLICE5:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_V5_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_HSLICE1:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H1_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_HSLICE2:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H2_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_HSLICE3:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H3_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_HSLICE4:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H4_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_HSLICE5:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_H5_SLICE,  selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_AREA1:      videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION1,   selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_AREA2:      videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION2,   selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_AREA3:      videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION3,   selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_AREA4:      videoWidget->displayMarkup( imageMarkup::MARKUP_ID_REGION4,   selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_PROFILE:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_LINE,      selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_TARGET:     videoWidget->displayMarkup( imageMarkup::MARKUP_ID_TARGET,    selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_BEAM:       videoWidget->displayMarkup( imageMarkup::MARKUP_ID_BEAM,      selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_TIMESTAMP:  videoWidget->displayMarkup( imageMarkup::MARKUP_ID_TIMESTAMP, selectedItem->isChecked() ); break;
        case imageContextMenu::ICM_DISPLAY_ELLIPSE:    videoWidget->displayMarkup( imageMarkup::MARKUP_ID_ELLIPSE,   selectedItem->isChecked() ); break;
    }
}



// Get the current selection option
QEImage::selectOptions QEImage::getSelectionOption()
{
    if( videoWidget->getPanning() )
    {
        return SO_PANNING;
    }
    else
    {
        switch( videoWidget->getMode() )
        {
        case imageMarkup::MARKUP_ID_V1_SLICE:  return SO_VSLICE1;
        case imageMarkup::MARKUP_ID_V2_SLICE:  return SO_VSLICE2;
        case imageMarkup::MARKUP_ID_V3_SLICE:  return SO_VSLICE3;
        case imageMarkup::MARKUP_ID_V4_SLICE:  return SO_VSLICE4;
        case imageMarkup::MARKUP_ID_V5_SLICE:  return SO_VSLICE5;
        case imageMarkup::MARKUP_ID_H1_SLICE:  return SO_HSLICE1;
        case imageMarkup::MARKUP_ID_H2_SLICE:  return SO_HSLICE2;
        case imageMarkup::MARKUP_ID_H3_SLICE:  return SO_HSLICE3;
        case imageMarkup::MARKUP_ID_H4_SLICE:  return SO_HSLICE4;
        case imageMarkup::MARKUP_ID_H5_SLICE:  return SO_HSLICE5;
        case imageMarkup::MARKUP_ID_REGION1:  return SO_AREA1;
        case imageMarkup::MARKUP_ID_REGION2:  return SO_AREA2;
        case imageMarkup::MARKUP_ID_REGION3:  return SO_AREA3;
        case imageMarkup::MARKUP_ID_REGION4:  return SO_AREA4;
        case imageMarkup::MARKUP_ID_LINE:     return SO_PROFILE;
        case imageMarkup::MARKUP_ID_TARGET:   return SO_TARGET;
        case imageMarkup::MARKUP_ID_BEAM:     return SO_BEAM;

        default:
        case imageMarkup::MARKUP_ID_NONE:    return SO_NONE;

        }
    }
}

//=================================================================================================
// Present information about the image.
// This is usefull when trying to determine why an image is not displaying well.
void QEImage::showImageAboutDialog()
{
    // Build the image information string
    QString about = QString ("QEImage image information:\n").append( iProcessor.getInfoText() );

// Note if mpeg stuff if included.
// To include mpeg stuff, don't define QE_USE_MPEG directly, define environment variable
// QE_FFMPEG to be processed by framework.pro
// Note: stub class returns a suitable non-url
    QString url = mpegSource->getURL();
    about.append( "\n\nImage MPEG URL: " ).append( (!url.isEmpty()) ? url :"No URL" );

    qcaobject::QCaObject *qca;

    qca = getQcaItem( IMAGE_VARIABLE );
    about.append( "\n\nImage data variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( FORMAT_VARIABLE );
    about.append( "\n\nImage format variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( BIT_DEPTH_VARIABLE );
    about.append( "\n\nBit depth variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( DATA_TYPE_VARIABLE );
    about.append( "\nData type variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( WIDTH_VARIABLE );
    about.append( "\n\nImage width variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( HEIGHT_VARIABLE );
    about.append( "\nImage height variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( NUM_DIMENSIONS_VARIABLE );
    about.append( "\n\nImage data dimensions variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( DIMENSION_0_VARIABLE );
    about.append( "\n\nImage dimension 1 variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( DIMENSION_1_VARIABLE );
    about.append( "\n\nImage dimension 2 variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    qca = getQcaItem( DIMENSION_2_VARIABLE );
    about.append( "\n\nImage dimension 3 variable: " ).append( (qca!=0)?qca->getRecordName():"No variable" );

    // Display the 'about' text
    QMessageBox::about(this, "About Image", about );
}

// Perform a named action.
// A call to this method originates from the application that created this widget and arrives with the
// support of the windowCustomisation class.
// The 'originator' QAction reference parameter is the QAction from the application's menu item or button
// generating this action request.
//
// For each action:
//
//     If initialising:
//         - If this widget will need to manipulate the application's menu item or button generating this
//           action request (such as disable it, or set its checked state), then save the 'originator' QAction
//           reference for future use.
//         - If the 'triggered' signal from the 'originator' QAction can be used directly, then connect to it.
//
//     If not initialising:
//         - Nothing may be required if already connected to the 'triggered' signal from the 'originator' QAction.
//         or
//         - Perform the action required.
//         - Optionally manipulate the 'originator' QAction. For example, enable it, disable it, modify its
//           label, or set its checked state as required. Note, if manipulating the 'originator' QAction
//           from within this function, the originator reference does not need to be saved when initialising
//           as it is passed in on each call.
void QEImage::actionRequest( QString action, QStringList /*arguments*/, bool initialise, QAction* originator )
{

    // Save button
    if( action == "Save...")
    {
        if( initialise )
        {
            QObject::connect(originator, SIGNAL(triggered()), this, SLOT(saveClicked()));
        }
    }

    // Pause button
    else if( action == "Pause")
    {
        if( initialise )
        {
            pauseExternalAction = originator;
            pauseExternalAction->setCheckable( true );
            QObject::connect(pauseExternalAction, SIGNAL(triggered()), this, SLOT(pauseClicked()));
        }
    }

    // Positioning button
    else if( action == "Move target position into beam")
    {
        if( initialise )
        {
            QObject::connect(originator, SIGNAL(triggered()), this, SLOT(targetClicked()));
        }
    }

    // About image button
    else if( action == "About image..." )
    {
        if( !initialise )
        {
            showImageAboutDialog();
        }
    }

    // Zoom menu
    else if( action == "Zoom" )
    {
        if( initialise )
        {
            originator->setMenu( zMenu );
        }
    }

    // Flip/Rotate menu
    else if( action == "Flip/Rotate" )
    {
        if( initialise )
        {
            originator->setMenu( frMenu );
        }
    }

    // Mode menu
    else if( action == "Mode" )
    {
        if( initialise )
        {
            originator->setMenu( sMenu );
        }
    }

    // Markup display menu
    else if( action == "Markup Display" )
    {
        if( initialise )
        {
            originator->setMenu( mdMenu );
        }
    }

    // Options dialog
    else if( action == "Options..." )
    {
        if( !initialise )
        {
            optionsDialog->exec( this );
        }
    }

    // Copy Image
    else if( action == "Copy" )
    {
        if( !initialise )
        {
            contextMenuTriggered( CM_COPY_DATA );
        }
    }

    // Launch Application 1
    else if( action == "LaunchApplication1" )
    {
        if( !initialise )
        {
            programLauncher1.launchImage( this, iProcessor.copyImage() );
        }
    }

    // Launch Application 2
    else if( action == "LaunchApplication2" )
    {
        if( !initialise )
        {
            programLauncher2.launchImage( this, iProcessor.copyImage() );
        }
    }

    // Show in fullscreen mode
    else if( action == "Full Screen" )
    {
        if( !initialise )
        {
            setFullScreen( true );
        }
    }

    // Unimplemented action
    else
    {
        sendMessage( QString( "QEImage widget has recieved the following unimplemented action request: ").append( action ));
    }

}

// Constructor for class used to hold a record of a single image
// Used when building a list of recorded images
historicImage::historicImage( QByteArray imageIn, unsigned long dataSizeIn, QCaAlarmInfo& alarmInfoIn, QCaDateTime& timeIn )
{
    image = imageIn;

    dataSize = dataSizeIn;
    alarmInfo = alarmInfoIn;
    time = timeIn;
}

// A configuration is being saved. Return any configuration to be saved for this widget
void QEImage::saveConfiguration( PersistanceManager* pm )
{
    // Prepare to save configuration for this widget
    const QString imageName = persistantName( "QEImage" );
    PMElement imageElement = pm->addNamedConfiguration( imageName );

    // Save the settings controlled by the image display properties dialog
    if( imageDisplayProps )
    {
        PMElement pvElement = imageElement.addElement( "DisplayProperties" );
        pvElement.addValue( "highPixel",              (int) (imageDisplayProps->getHighPixel()) );
        pvElement.addValue( "lowPixel",               (int) (imageDisplayProps->getLowPixel()) );
        pvElement.addValue( "autoBrightnessContrast", (bool)(imageDisplayProps->getAutoBrightnessContrast()) );
        pvElement.addValue( "contrastReversal",       (bool)(imageDisplayProps->getContrastReversal()) );
        pvElement.addValue( "falseColour",            (bool)(imageDisplayProps->getFalseColour()) );
        pvElement.addValue( "histZoom",               (int) (imageDisplayProps->getHistZoom()) );
        pvElement.addValue( "log",                    (bool)(imageDisplayProps->getLog()) );
    }

    // Save markup information
    PMElement pvElement = imageElement.addElement( "Markups" );
    pvElement.addValue( "enableHozSlicePresentation",   (bool)(getEnableHozSlicePresentation()) );
    pvElement.addValue( "enableVertSlicePresentation",  (bool)(getEnableVertSlicePresentation()) );
    pvElement.addValue( "enableProfilePresentation",    (bool)(getEnableProfilePresentation()) );
    pvElement.addValue( "enableArea1Selection",         (bool)(getEnableArea1Selection()) );
    pvElement.addValue( "enableArea2Selection",         (bool)(getEnableArea2Selection()) );
    pvElement.addValue( "enableArea3Selection",         (bool)(getEnableArea3Selection()) );
    pvElement.addValue( "enableArea4Selection",         (bool)(getEnableArea4Selection()) );
    pvElement.addValue( "enableTargetSelection",        (bool)(getEnableTargetSelection()) );
    pvElement.addValue( "enableBeamSelection",          (bool)(getEnableBeamSelection()) );

    pvElement.addValue( "displayVertSlice1Selection",   (bool)(getDisplayVertSlice1Selection()) );
    pvElement.addValue( "displayVertSlice2Selection",   (bool)(getDisplayVertSlice2Selection()) );
    pvElement.addValue( "displayVertSlice3Selection",   (bool)(getDisplayVertSlice3Selection()) );
    pvElement.addValue( "displayVertSlice4Selection",   (bool)(getDisplayVertSlice4Selection()) );
    pvElement.addValue( "displayVertSlice5Selection",   (bool)(getDisplayVertSlice5Selection()) );
    pvElement.addValue( "displayHozSlice1Selection",    (bool)(getDisplayHozSlice1Selection()) );
    pvElement.addValue( "displayHozSlice2Selection",    (bool)(getDisplayHozSlice2Selection()) );
    pvElement.addValue( "displayHozSlice3Selection",    (bool)(getDisplayHozSlice3Selection()) );
    pvElement.addValue( "displayHozSlice4Selection",    (bool)(getDisplayHozSlice4Selection()) );
    pvElement.addValue( "displayHozSlice5Selection",    (bool)(getDisplayHozSlice5Selection()) );
    pvElement.addValue( "displayProfileSelection",      (bool)(getDisplayProfileSelection()) );
    pvElement.addValue( "displayArea1Selection",        (bool)(getDisplayArea1Selection()) );
    pvElement.addValue( "displayArea2Selection",        (bool)(getDisplayArea2Selection()) );
    pvElement.addValue( "displayArea3Selection",        (bool)(getDisplayArea3Selection()) );
    pvElement.addValue( "displayArea4Selection",        (bool)(getDisplayArea4Selection()) );
    pvElement.addValue( "displayTargetSelection",       (bool)(getDisplayTargetSelection()) );
    pvElement.addValue( "displayBeamSelection",         (bool)(getDisplayBeamSelection()) );
    pvElement.addValue( "displayEllipse",               (bool)(getDisplayEllipse()) );


    // Save other attributes of the image under the user's control
    pvElement = imageElement.addElement( "View" );
    pvElement.addValue( "displayButtonBar",             (bool)(getDisplayButtonBar()) );
    pvElement.addValue( "showTime",                     (bool)(getShowTime()) );
    pvElement.addValue( "verticalFlip",                 (bool)(getVerticalFlip()) );
    pvElement.addValue( "horizontalFlip",               (bool)(getHorizontalFlip()) );
    pvElement.addValue( "enableImageDisplayProperties", (bool)(getEnableImageDisplayProperties()) );
    pvElement.addValue( "enableRecording",              (bool)(getEnableRecording()) );
    pvElement.addValue( "zoom",                         (int) (getZoom()) );
    pvElement.addValue( "hozScroll",                    (int) (scrollArea->horizontalScrollBar()->value()) );
    pvElement.addValue( "vertScroll",                   (int) (scrollArea->verticalScrollBar()  ->value()) );
}

//------------------------------------------------------------------------------
// A configuration is being restored. Use any configuration information saved by this widget
void QEImage::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
    // Do nothing during the framework phase
    if( restorePhase != FRAMEWORK )
    {
        return;
    }

    // Retrieve our configuration
    const QString imageName = persistantName( "QEImage" );
    PMElement imageElement = pm->getNamedConfiguration( imageName );

    // If there is an image properties dialog, restore the setting it is used to control
    if( imageDisplayProps )
    {
        PMElement pvElement = imageElement.getElement( "DisplayProperties" );
        if( !pvElement.isNull() )
        {
            bool status;
            bool bval;
            int  ival;

            // Set the brightness/contrast.
            // High and low pixel values are set as a pair. Ensure we have both - either from the configuration, or the current values
            int highPixel, lowPixel;
            status = pvElement.getValue( "highPixel", highPixel );
            if( !status )
            {
                highPixel = imageDisplayProps->getHighPixel();
            }
            status = pvElement.getValue( "lowPixel", lowPixel   );
            if( !status )
            {
                lowPixel  = imageDisplayProps->getLowPixel();
            }
            imageDisplayProps->setBrightnessContrast( highPixel, lowPixel );

            // Set other image display properties
            status = pvElement.getValue( "autoBrightnessContrast", bval ); if( status ) { imageDisplayProps->setAutoBrightnessContrast( bval ); }
            status = pvElement.getValue( "contrastReversal",       bval ); if( status ) { imageDisplayProps->setContrastReversal(       bval ); }
            status = pvElement.getValue( "falseColour",            bval ); if( status ) { imageDisplayProps->setFalseColour(            bval ); }
            status = pvElement.getValue( "histZoom",               ival ); if( status ) { imageDisplayProps->setHistZoom(               ival ); }
            status = pvElement.getValue( "log",                    bval ); if( status ) { imageDisplayProps->setLog(                    bval ); }

            // Apply the changes to the image display properties dialog
            imageDisplayPropertiesChanged();
        }
    }

    // Restore markup settings
    PMElement pvElement = imageElement.getElement( "Markups" );
    if( !pvElement.isNull() )
    {
        bool status;
        bool bval;

        status = pvElement.getValue( "enableHozSlicePresentation",  bval ); if( status ) { setEnableHozSlicePresentation(  bval ); }
        status = pvElement.getValue( "enableVertSlicePresentation", bval ); if( status ) { setEnableVertSlicePresentation( bval ); }
        status = pvElement.getValue( "enableProfilePresentation",   bval ); if( status ) { setEnableProfilePresentation(   bval ); }
        status = pvElement.getValue( "enableArea1Selection",        bval ); if( status ) { setEnableArea1Selection(        bval ); }
        status = pvElement.getValue( "enableArea2Selection",        bval ); if( status ) { setEnableArea2Selection(        bval ); }
        status = pvElement.getValue( "enableArea3Selection",        bval ); if( status ) { setEnableArea3Selection(        bval ); }
        status = pvElement.getValue( "enableArea4Selection",        bval ); if( status ) { setEnableArea4Selection(        bval ); }
        status = pvElement.getValue( "enableTargetSelection",       bval ); if( status ) { setEnableTargetSelection(       bval ); }
        status = pvElement.getValue( "enableBeamSelection",         bval ); if( status ) { setEnableBeamSelection(         bval ); }

        status = pvElement.getValue( "displayVertSliceSelection",   bval ); if( status ) { setDisplayVertSlice1Selection(   bval ); } // For backward compatibility with before aditional slices
        status = pvElement.getValue( "displayVertSlice1Selection",  bval ); if( status ) { setDisplayVertSlice1Selection(   bval ); }
        status = pvElement.getValue( "displayVertSlice2Selection",  bval ); if( status ) { setDisplayVertSlice2Selection(   bval ); }
        status = pvElement.getValue( "displayVertSlice3Selection",  bval ); if( status ) { setDisplayVertSlice3Selection(   bval ); }
        status = pvElement.getValue( "displayVertSlice4Selection",  bval ); if( status ) { setDisplayVertSlice4Selection(   bval ); }
        status = pvElement.getValue( "displayVertSlice5Selection",  bval ); if( status ) { setDisplayVertSlice5Selection(   bval ); }
        status = pvElement.getValue( "displayHozSliceSelection",    bval ); if( status ) { setDisplayHozSlice1Selection(    bval ); } // For backward compatibility with before aditional slices
        status = pvElement.getValue( "displayHozSlice1Selection",   bval ); if( status ) { setDisplayHozSlice1Selection(    bval ); }
        status = pvElement.getValue( "displayHozSlice2Selection",   bval ); if( status ) { setDisplayHozSlice2Selection(    bval ); }
        status = pvElement.getValue( "displayHozSlice3Selection",   bval ); if( status ) { setDisplayHozSlice3Selection(    bval ); }
        status = pvElement.getValue( "displayHozSlice4Selection",   bval ); if( status ) { setDisplayHozSlice4Selection(    bval ); }
        status = pvElement.getValue( "displayHozSlice5Selection",   bval ); if( status ) { setDisplayHozSlice5Selection(    bval ); }
        status = pvElement.getValue( "displayProfileSelection",     bval ); if( status ) { setDisplayProfileSelection(     bval ); }
        status = pvElement.getValue( "displayArea1Selection",       bval ); if( status ) { setDisplayArea1Selection(       bval ); }
        status = pvElement.getValue( "displayArea2Selection",       bval ); if( status ) { setDisplayArea2Selection(       bval ); }
        status = pvElement.getValue( "displayArea3Selection",       bval ); if( status ) { setDisplayArea3Selection(       bval ); }
        status = pvElement.getValue( "displayArea4Selection",       bval ); if( status ) { setDisplayArea4Selection(       bval ); }
        status = pvElement.getValue( "displayTargetSelection",      bval ); if( status ) { setDisplayTargetSelection(      bval ); }
        status = pvElement.getValue( "displayBeamSelection",        bval ); if( status ) { setDisplayBeamSelection(        bval ); }
        status = pvElement.getValue( "displayEllipse",              bval ); if( status ) { setDisplayEllipse(              bval ); }
    }

    // Restore current view
    // These include all the attributes the user can alter without a GUI redesign
    pvElement = imageElement.getElement( "View" );
    if( !pvElement.isNull() )
    {
        bool status;
        bool bval;
        int  ival;

        status = pvElement.getValue( "displayButtonBar",             bval ); if( status ) { setDisplayButtonBar(             bval ); }
        status = pvElement.getValue( "showTime",                     bval ); if( status ) { setShowTime(                     bval ); }
        status = pvElement.getValue( "verticalFlip",                 bval ); if( status ) { setVerticalFlip(                 bval ); }
        status = pvElement.getValue( "horizontalFlip",               bval ); if( status ) { setHorizontalFlip(               bval ); }
        status = pvElement.getValue( "enableImageDisplayProperties", bval ); if( status ) { setEnableImageDisplayProperties( bval ); }
        status = pvElement.getValue( "enableRecording",              bval ); if( status ) { setEnableRecording(              bval ); }
        status = pvElement.getValue( "zoom",                         ival ); if( status ) { setZoom(                         ival ); }
        status = pvElement.getValue( "hozScroll",                    ival ); if( status ) { setInitialVertScrollPos(         ival ); }
        status = pvElement.getValue( "vertScroll",                   ival ); if( status ) { setInitialHozScrollPos(          ival ); }

        // Ensure scroll bars are set when the image is next displayed.
        // The calls to setInitialVertScrollPos() and setInitialHozScrollPos() above save and set the scroll bar
        // position, but the set won't work as the scroll bars have not been realised yet. The set won't work
        // unless the scroll bar is shown and its geometry has been sorted.
        initScrollPosSet = false;
    }
}

// end
