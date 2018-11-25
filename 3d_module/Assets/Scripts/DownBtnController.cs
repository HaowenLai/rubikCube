using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class DownBtnController : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData pointerEventData)
    {
        if (pointerEventData.button == PointerEventData.InputButton.Left)
            CubeController.turnDownFace(90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Right)
            CubeController.turnDownFace(-90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Middle)
            CubeController.turnDownFace(180.0f);
    }
}
