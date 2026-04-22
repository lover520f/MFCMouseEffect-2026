import DialogHost from '../dialog/DialogHost.svelte';
import { syncMountedComponent } from './component-instance.js';

const mountNode = document.createElement('div');
document.body.appendChild(mountNode);

function createDialogComponent(target, props) {
  return new DialogHost({
    target,
    props: {
      ...props,
      onConfirmAction: () => {
        closeWithResult(true);
      },
      onCancelAction: () => {
        closeWithResult(false);
      },
      onMaskAction: () => {
        if (dialogMode === 'confirm') {
          closeWithResult(false);
          return;
        }
        removeDialog({ settlePending: false });
      },
    },
  });
}

let component = createDialogComponent(mountNode, {
  open: false,
  mode: 'notice',
  title: '',
  message: '',
  okText: 'OK',
  cancelText: 'Cancel',
});

let dialogMode = 'notice';
let isOpen = false;
let onClose = null;
let onResolve = null;

function syncBodyModalClass(open) {
  if (open) {
    document.body.classList.add('mfx-modal-open');
    return;
  }
  document.body.classList.remove('mfx-modal-open');
}

function detachEsc() {
  document.removeEventListener('keydown', handleEsc);
}

function attachEsc() {
  document.addEventListener('keydown', handleEsc);
}

function setDialogState(next) {
  component = syncMountedComponent(component, mountNode, createDialogComponent, next);
}

function removeDialog(options) {
  const opts = options || {};
  const settlePending = opts.settlePending === true;
  const pendingResult = Object.prototype.hasOwnProperty.call(opts, 'pendingResult')
    ? opts.pendingResult
    : false;
  const hadDialog = isOpen || !!onClose || !!onResolve;

  detachEsc();
  syncBodyModalClass(false);
  isOpen = false;
  dialogMode = 'notice';
  setDialogState({ open: false });

  if (settlePending && onResolve) {
    const resolve = onResolve;
    onResolve = null;
    resolve(pendingResult);
  } else {
    onResolve = null;
  }

  const close = onClose;
  onClose = null;
  if (hadDialog && close) close();
}

function closeWithResult(result) {
  const resolve = onResolve;
  onResolve = null;
  removeDialog({ settlePending: false });
  if (resolve) resolve(result);
}

function handleEsc(event) {
  if (event.key !== 'Escape') return;
  closeWithResult(false);
}

function showNotice(opts) {
  const options = opts || {};
  removeDialog({ settlePending: true, pendingResult: false });

  dialogMode = 'notice';
  onClose = options.onClose || null;
  isOpen = true;
  attachEsc();
  syncBodyModalClass(true);
  setDialogState({
    open: true,
    mode: 'notice',
    title: options.title || 'Notice',
    message: options.message || '',
    okText: options.okText || 'OK',
    cancelText: options.cancelText || 'Cancel',
  });
}

function showConfirm(opts) {
  const options = opts || {};
  return new Promise((resolve) => {
    removeDialog({ settlePending: true, pendingResult: false });

    dialogMode = 'confirm';
    onClose = options.onClose || null;
    onResolve = resolve;
    isOpen = true;
    attachEsc();
    syncBodyModalClass(true);
    setDialogState({
      open: true,
      mode: 'confirm',
      title: options.title || 'Confirm',
      message: options.message || '',
      okText: options.okText || 'OK',
      cancelText: options.cancelText || 'Cancel',
    });
  });
}

window.MfxDialog = {
  showNotice,
  showConfirm,
};
