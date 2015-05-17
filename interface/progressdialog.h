#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

public slots:
    void advanceToNextStage();
    void setStageMaximum(unsigned max);
    void updateProgress(unsigned current);

private:
    Ui::ProgressDialog *ui;
    std::vector<unsigned> stage_progress;
    unsigned current_stage;
    unsigned stage_maximum;

    QLabel* getLabel(unsigned i);
};

#endif // PROGRESSDIALOG_H
