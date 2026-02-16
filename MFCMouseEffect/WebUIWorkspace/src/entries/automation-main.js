import AutomationEditor from '../automation/AutomationEditor.svelte';
import { createAutomationApi } from '../automation/api.js';

window.MfxAutomationUi = createAutomationApi(AutomationEditor, 'automation_editor_mount');
