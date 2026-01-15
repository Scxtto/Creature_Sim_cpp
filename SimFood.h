#pragma once

/**
 * @brief Food entity available for consumption.
 */
class Food {
public:
    /**
     * @brief Create a food item at the given position with energy content.
     * @param id Unique food id.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param energyContent Energy value granted on consumption.
     */
    Food(int id, double x, double y, double energyContent);

    /**
     * @brief Advance lifetime and mark consumed if expired.
     * @note Decrements the internal duration counter.
     */
    void update();

    /** @brief Food id. */
    int id() const { return m_id; }
    /** @brief X coordinate. */
    double x() const { return m_x; }
    /** @brief Y coordinate. */
    double y() const { return m_y; }
    /** @brief Render size in pixels. */
    double size() const { return m_size; }
    /** @brief Energy value for consuming. */
    double energyContent() const { return m_energyContent; }
    /** @brief True when food is consumed or expired. */
    bool consumed() const { return m_consumed; }

    /**
     * @brief Mark the food as consumed and clear energy.
     * @note After this call \c consumed() returns true.
     */
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
