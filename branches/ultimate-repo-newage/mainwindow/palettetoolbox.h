/** @file palettetoolbox.h
 * 	@brief Класс палитры элементов
 * */
#pragma once
 
#include <QTabWidget>
#include "../common/realrepoapiclasses.h"

class QDragEnterEvent;
class QDropEvent;

/** @class PaletteToolbox
 * 	@brief Класс палитры элементов
 * */
class PaletteToolbox : public QTabWidget
{
	public:
		explicit PaletteToolbox(QWidget *parent = 0);
		~PaletteToolbox();

		void setEditors(QVector<bool> const &editors);
		QVector<bool> getSelectedTabs() const;
	protected:
		/** @brief Обработать событие начала операции drag'n'drop */
		void dragEnterEvent(QDragEnterEvent *event /**< Событие */);
		/** @brief Обработать событие завершения операции drag'n'drop*/
		void dropEvent(QDropEvent *event /**< Событие */);
		/** @brief Обработать событие нажатия кнопки мыши*/
		void mousePressEvent(QMouseEvent *event /**< Событие*/);
		virtual void wheelEvent(QWheelEvent *event);

	private:
		/** @class DraggableElement
		 * 	@brief Элемент палитры объектов */
		class DraggableElement : public QWidget {
			public:
				DraggableElement(TypeIdType const &classid, QWidget *parent = NULL);
				/** @brief Получить икноку элемента
				 * 	@brief @return Иконка элемента
				 * 	*/
				QIcon icon()
				{
					return m_icon;
				}
				/** @brief Получить надпись элемента
				 *	@brief @return Надпись элемента
				 * */
				QString text()
				{
					return m_text;
				}
				/** @brief Получить идентификатор элемента
				 *	@brief @return Идентификатор элемента
				 * */
				TypeIdType id()
				{
					return m_id;
				}
			private:
				/** @brief Идентификатор элемента */
				TypeIdType m_id;
				/** @brief Иконка элемента */
				QIcon m_icon;
				/** @brief Надпись элемента */
				QString m_text;
		};

		void checkFirstLaunch();

		QVector<QWidget*> mTabs;
		QVector<QString> mTabNames;
		QVector<bool> mShownTabs;
};