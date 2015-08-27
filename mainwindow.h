#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdint.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool updateGeneralCfg(void);
    bool updateIOModuleCfg(void);
    bool updateJsn2Cfg(void);
    bool updateMeterCfg(void);
    bool updateGeneralWeatherMeasure(void);
    bool updateLockAutomCfg(void);
    bool updateIoCfg(void);
    bool updateModbusSlaveCfg(void);
    bool updateEthCfg(void);
    bool updateRsCfg(void);

    void setCfgStructs(char* buf);
    bool setGeneralCfg(void);
    bool setIOModuleCfg(void);
    bool setJsn2Cfg(void);
    bool setMeterCfg(void);
    bool setGeneralWeatherMeasure(void);
    bool setWeatherAutomCfg(int id);
    bool setLockAutomCfg(void);
    bool setCircuitCfg(int id);
    bool setIoCfg(void);
    bool setModbusSlaveCfg(void);
    bool setEthCfg(void);
    bool setRsCfg(void);

    int checkIoMod(uint8_t module_id, uint8_t input_output_type, QString text);

private slots:
    void on_actionOtw_rz_triggered();
    void on_actionZako_cz_triggered();
    void on_actionZapisz_triggered();
    void on_addIoModuleBtn_clicked();
    void on_delIoModuleBtn_clicked();
    void on_addJsn2Btn_clicked();
    void on_delJsn2Btn_clicked();
    void on_addMeterBtn_clicked();
    void on_delMeterBtn_clicked();

    void on_generalTempSensorTypeCmb_currentIndexChanged(int index);

    void on_generalFallSensorTypeCmb_currentIndexChanged(int index);

    void on_weatherAutomList_currentRowChanged(int currentRow);

    void on_editWeatherAutomBtn_clicked();

    void on_weatherAutomCount_valueChanged(int arg1);

    void on_sensorColdTypeCmb_currentIndexChanged(int index);

    void on_sensorHotTypeCmb_currentIndexChanged(int index);

    void on_blowSensTypeCmb_currentIndexChanged(int index);

    void on_cirCount_valueChanged(int arg1);

    void on_editCircuitList_clicked();

    void on_circuitList_currentRowChanged(int currentRow);

    void on_modbusSlaveMediumCmb_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
