#ifndef UITABLEVIEW_H
#define UITABLEVIEW_H

#include <QTableView>

class UITableView : public QTableView
{
	Q_OBJECT

public:
	UITableView(QWidget *parent);
	~UITableView();

//slots:
//	void UpdateCheck(const Qt::CheckState checkState);
//
private:
	
};

#endif // UITABLEVIEW_H
