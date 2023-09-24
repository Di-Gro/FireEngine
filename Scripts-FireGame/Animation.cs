using System;
using Engine;

class Animation
{
    /* Текущее время - прошло со старта */
    /* Целевое время -  */
    /* Для пикапа - анимация рук вниз, потом вызвать пикап, который в игроке. (2 анимации)
     * После пикапа, запустить вторую анимацию. С Drop аналогично.
     * 
     * Когда кидаем предмет, обнаружить столкновение
     */
    private bool m_needUpdate = false;

    public bool NeedUpdate()
    {
        if (m_needUpdate)
            return true;
        else
            return false;
    }
}
