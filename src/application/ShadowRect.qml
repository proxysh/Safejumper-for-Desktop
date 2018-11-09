import QtQuick 2.11
import QtGraphicalEffects 1.0

Rectangle {
    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: 2
        radius: 10
        samples: 21
        color: "#0A000000"
    }
}
