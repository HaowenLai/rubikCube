using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class BackBtnController : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData pointerEventData)
    {
        if (pointerEventData.button == PointerEventData.InputButton.Left)
            CubeController.turnBackFace(90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Right)
            CubeController.turnBackFace(-90.0f);
        else if (pointerEventData.button == PointerEventData.InputButton.Middle)
            CubeController.turnBackFace(180.0f);
    }
}
