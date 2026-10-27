
const API = 'https://contact-management-system-7zfn.onrender.com';

const rows = document.getElementById('contactRows');
const modal = document.getElementById('modal');
const form = document.getElementById('contactForm');
const notice = document.getElementById('notice');
const searchInput = document.getElementById('searchInput');

let contacts = [];
let currentView = 'all';
let currentCategory = '';
let editingId = null;
let noticeTimer;

/* Send a request to the C backend */
async function request(path, options = {}) {
  const response = await fetch(API + path, options);
  const data = await response.json();

  if (!response.ok || data.success === false) {
    throw new Error(data.message || 'Request failed');
  }

  return data;
}

/* Send form values to the backend */
function post(path, values) {
  return request(path, {
    method: 'POST',
    headers: {
      'Content-Type':
        'application/x-www-form-urlencoded;charset=UTF-8'
    },
    body: new URLSearchParams(values).toString()
  });
}

/* Show success or error messages */
function showNotice(message, type = 'success') {
  clearTimeout(noticeTimer);

  notice.textContent = message;
  notice.className = `notice show ${type}`;

  noticeTimer = setTimeout(() => {
    notice.className = 'notice';
  }, 3500);
}

/* Get initials for a contact avatar */
function initials(name) {
  return (name || '?')
    .trim()
    .split(/\s+/)
    .slice(0, 2)
    .map(part => part[0] || '')
    .join('')
    .toUpperCase();
}

/* Escape text before displaying it as HTML */
function escapeHtml(value) {
  return String(value ?? '').replace(/[&<>"']/g, char => ({
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;'
  })[char]);
}

/* Highlight the active sidebar item */
function setActive(button) {
  document.querySelectorAll('.nav').forEach(item => {
    item.classList.remove('active');
  });

  button.classList.add('active');
}

/* Update dashboard statistics */
function updateStats() {
  document.getElementById('totalCount').textContent =
    contacts.length;

  document.getElementById('categoryCount').textContent =
    new Set(contacts.map(c => c.category)).size;
}

/* Apply search and category filters */
function visibleContacts() {
  let list = [...contacts];

  if (currentCategory) {
    list = list.filter(contact =>
      contact.category.toLowerCase() ===
      currentCategory.toLowerCase()
    );
  }

  const query = searchInput.value.trim().toLowerCase();

  if (query) {
    list = list.filter(contact =>
      contact.name.toLowerCase().includes(query) ||
      contact.phone.toLowerCase().includes(query) ||
      contact.id.toLowerCase().includes(query)
    );
  }

  if (currentView === 'sort') {
    list.sort((a, b) => a.name.localeCompare(b.name));
  }

  return list;
}

/* Draw the contact table */
function render() {
  const list = visibleContacts();

  rows.innerHTML = list.map(contact => `
    <tr>
      <td>
        <div class="person">
          <div class="person-avatar">
            ${escapeHtml(initials(contact.name))}
          </div>
          <div>
            <div class="person-name">
              ${escapeHtml(contact.name)}
            </div>
            <div class="person-id">
              ${escapeHtml(contact.id)}
            </div>
          </div>
        </div>
      </td>

      <td class="phone">${escapeHtml(contact.phone)}</td>

      <td>
        <span class="tag ${escapeHtml(
          contact.category.toLowerCase()
        )}">
          ${escapeHtml(contact.category)}
        </span>
      </td>

      <td>${escapeHtml(contact.email || '—')}</td>

      <td class="actions">
        <button class="action"
                data-edit="${escapeHtml(contact.id)}">
          Edit
        </button>

        <button class="action delete"
                data-delete="${escapeHtml(contact.id)}">
          Delete
        </button>
      </td>
    </tr>
  `).join('');

  const empty = list.length === 0;

  document.getElementById('emptyState').style.display =
    empty ? 'block' : 'none';

  document.querySelector('.table-wrap').style.display =
    empty ? 'none' : 'block';

  document.getElementById('resultCount').textContent =
    `${list.length} contact${list.length === 1 ? '' : 's'}`;

  updateStats();
}

/* Load contacts from the C backend */
async function loadContacts() {
  try {
    const data = await request('/api/contacts');
    contacts = data.contacts || [];
    render();
  } catch (error) {
    showNotice(
      `${error.message}. Make sure the C server is running.`,
      'error'
    );
  }
}

/* Open the add/edit form */
function openModal(contact = null) {
  editingId = contact ? contact.id : null;

  form.reset();

  document.getElementById('modalTitle').textContent =
    contact ? 'Edit contact' : 'Add contact';

  document.getElementById('saveBtn').textContent =
    contact ? 'Save changes' : 'Save contact';

  if (contact) {
    for (const key of [
      'name', 'phone', 'email', 'address', 'category'
    ]) {
      form.elements[key].value = contact[key] || '';
    }
  }

  modal.classList.remove('hidden');

  setTimeout(() => {
    form.elements.name.focus();
  }, 50);
}

/* Close the form */
function closeModal() {
  modal.classList.add('hidden');
  editingId = null;
}

/* Open form buttons */
document.getElementById('addBtn')
  .addEventListener('click', () => openModal());

document.getElementById('emptyAddBtn')
  .addEventListener('click', () => openModal());

document.getElementById('closeModal')
  .addEventListener('click', closeModal);

document.getElementById('cancelModal')
  .addEventListener('click', closeModal);

modal.addEventListener('click', event => {
  if (event.target === modal) {
    closeModal();
  }
});

document.addEventListener('keydown', event => {
  if (event.key === 'Escape') {
    closeModal();
  }
});

/* Add or update a contact */
form.addEventListener('submit', async event => {
  event.preventDefault();

  const name = form.elements.name.value.trim();
  const phone = form.elements.phone.value.trim();

  if (!name || !phone) {
    showNotice(
      'Full name and phone number are required.',
      'error'
    );
    return;
  }

  const values = Object.fromEntries(
    new FormData(form).entries()
  );

  values.name = name;
  values.phone = phone;

  // New contacts receive their ID from the C backend.
  // Editing requires the existing ID.
  if (editingId) {
    values.id = editingId;
  }

  const wasEditing = Boolean(editingId);
  const saveButton = document.getElementById('saveBtn');

  saveButton.disabled = true;

  try {
    const result = wasEditing
      ? await post('/api/update', values)
      : await post('/api/add', values);

    closeModal();
    await loadContacts();

    showNotice(
      result.message ||
      (wasEditing
        ? 'Contact updated successfully.'
        : 'Contact added successfully.')
    );
  } catch (error) {
    showNotice(error.message, 'error');
  } finally {
    saveButton.disabled = false;
  }
});

/* Handle edit and delete buttons */
rows.addEventListener('click', async event => {
  const editButton = event.target.closest('[data-edit]');
  const deleteButton = event.target.closest('[data-delete]');

  if (editButton) {
    const contact = contacts.find(
      item => item.id === editButton.dataset.edit
    );

    if (contact) {
      openModal(contact);
    }
  }

  if (deleteButton) {
    const id = deleteButton.dataset.delete;

    const contact = contacts.find(item => item.id === id);

    if (!contact) {
      return;
    }

    if (!confirm(
      `Delete ${contact.name}? This cannot be undone.`
    )) {
      return;
    }

    try {
      const result = await post('/api/delete', { id });

      await loadContacts();
      showNotice(result.message || 'Contact deleted.');
    } catch (error) {
      showNotice(error.message, 'error');
    }
  }
});

/* Live search */
searchInput.addEventListener('input', render);

/* Clear search */
document.getElementById('clearSearch')
  .addEventListener('click', () => {
    searchInput.value = '';
    render();
    searchInput.focus();
  });

/* All contacts and alphabetical order */
document.querySelectorAll('[data-view]')
  .forEach(button => {
    button.addEventListener('click', () => {
      currentView = button.dataset.view;
      currentCategory = '';

      setActive(button);

      document.getElementById('listTitle').textContent =
        currentView === 'sort'
          ? 'Alphabetical order'
          : 'All contacts';

      document.getElementById('listSubtitle').textContent =
        currentView === 'sort'
          ? 'Contacts sorted by name.'
          : 'Manage your personal and professional connections.';

      render();
    });
  });

/* Category filters */
document.querySelectorAll('[data-category]')
  .forEach(button => {
    button.addEventListener('click', () => {
      currentCategory = button.dataset.category;
      currentView = 'all';

      setActive(button);

      document.getElementById('listTitle').textContent =
        `${currentCategory} contacts`;

      document.getElementById('listSubtitle').textContent =
        `Contacts grouped under ${currentCategory}.`;

      render();
    });
  });

/* Load the 15 seeded contacts from C */
loadContacts();
