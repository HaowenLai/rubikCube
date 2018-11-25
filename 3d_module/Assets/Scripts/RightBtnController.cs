using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class RightBtnController : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData pointerEventData)
    {
        if (pointerEventData.button == PointerEventData.InputButton.Left)
            CubeController.turnRightFace(90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Right)
            CubeController.turnRightFace(-90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Middle)
            CubeController.turnRightFace(180.0f);
    }
}
