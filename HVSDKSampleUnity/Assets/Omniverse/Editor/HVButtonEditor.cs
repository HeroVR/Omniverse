using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEditor.UI;
// Custom Editor the "old" way by modifying the script variables directly.
// No handling of multi-object editing, undo, and prefab overrides!
[CanEditMultipleObjects, CustomEditor(typeof(HVButton), true)]
public class HVButtonEditor : SelectableEditor
{
    private SerializedProperty m_OnClickProperty;

    private SerializedProperty m_InteractableProperty;

    [MenuItem("Examples/Editor GUILayout Toggle Usage")]
    protected override void OnEnable()
    {
        this.m_OnClickProperty = base.serializedObject.FindProperty("m_OnClick");
    }

    public override void OnInspectorGUI()
    {
        HVButton hb = (HVButton)target;
        hb._isCheckBox = (bool)EditorGUILayout.Toggle("_isCheckBox", hb._isCheckBox);
        hb._StyleNormal = (Sprite)EditorGUILayout.ObjectField("_StyleNormal", hb._StyleNormal, typeof(Sprite), true);

        // EditorGUILayout.PropertyField(this.m_InteractableProperty, new GUILayoutOption[0]);
        EditorGUILayout.Space();//空行
        base.serializedObject.Update();
        EditorGUILayout.PropertyField(this.m_OnClickProperty, new GUILayoutOption[0]);
        base.serializedObject.ApplyModifiedProperties();
    }
}
