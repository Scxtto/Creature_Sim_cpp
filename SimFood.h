#pragma once

class Food {
public:
    Food(int id, double x, double y, double energyContent);

    void update();

    int id() const { return m_id; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    double size() const { return m_size; }
    double energyContent() const { return m_energyContent; }
    bool consumed() const { return m_consumed; }

    void markConsumed();

private:
    int m_id = 0;
    double m_x = 0.0;
    double m_y = 0.0;
    double m_size = 3.0;
    double m_energyContent = 0.0;
    bool m_consumed = false;
    int m_duration = 500;
};
